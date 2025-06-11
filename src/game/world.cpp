#include "world.h"

#include "engine/game_context.h"
#include "engine/image_asset.h"
#include "engine/gfx.h"

#include "collision.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>

#include <simdjson.h>
using namespace simdjson;

SDL_Rect LdtkLevel::get_bounding_box() {
	return { pxWorldX, pxWorldY, pxWidth, pxHeight };
}

static inline const char* copy_to_arena(std::string_view str, mems::Arena& arena) {
	const size_t len = str.length();
	char* buf = static_cast<char*>(arena.push(len + 1)	);
	str.copy(buf, len);
	buf[len] = 0;
	return buf;
}

// get parent directory path from filepath
// NOTE(sand): the directory we are producing here will have a path separator at the end
//             this function also assumes that GameWorld::arena has been initialized
const char* GameWorld::_get_parent_dir(const char* path) {
	mems::Arena& scratch = mems::get_scratch();
	mems::ArenaScope scope(scratch);

	// calculate path length, and trim whitespace from right end
	size_t pathLen = strlen(path);
	for (; (pathLen - 1) >= 0; pathLen--) {
		if (path[pathLen - 1] != ' ')
			break;
	}

	char* mutPath = static_cast<char*>(scratch.push(pathLen));
	memcpy(mutPath, path, pathLen);

	// determine path separator
	// if path has a backslash we'll assume that \ is the path separator
	char pathSeparator = '/';
	for (int i = 0; i < pathLen; i++) {
		if (mutPath[i] == '\\') {
			pathSeparator = '\\';
			break;
		}
	}

	// append pathSeparator if it is not at the end
	if (mutPath[pathLen - 1] != pathSeparator) {
		scratch.push(1);
		mutPath[pathLen++] = pathSeparator;
	}

	// append ../ and null terminator to mutPath and return
	char* returnPath = static_cast<char*>(arena.push(pathLen + 3 + 1));
	memcpy(returnPath, mutPath, pathLen);
	returnPath[pathLen] = '.';
	returnPath[pathLen + 1] = '.';
	returnPath[pathLen + 2] = pathSeparator;
	returnPath[pathLen + 3] = 0;
	return returnPath;
}

void GameWorld::init(const char* path, GameContext& ctx) {
	arena.alloc();
	parentDirPath = _get_parent_dir(path);
	padded_string jsonString = padded_string::load(path);
	simdjson::ondemand::document doc = GET_JSON_PARSER->iterate(jsonString);

	//
	// loading definitions
	// we only care about tilesets (see https://ldtk.io/json/#ldtk-DefinitionsJson)
	auto tilesetDefinitions = doc["defs"]["tilesets"].get_array();
	nTilesets = static_cast<int>(tilesetDefinitions.count_elements());
	tilesets = static_cast<LdtkTilesetDef*>(arena.push_zero(sizeof(LdtkTilesetDef) * nTilesets));
	int i = 0;

#define LOAD_STRING()

	for (auto tilesetDef : tilesetDefinitions) {
		LdtkTilesetDef& def = tilesets[i++];

		// NOTE(sand): take care to read these in the same order they are in the json document
		def.widthCells = static_cast<int>(tilesetDef["__cWid"]);
		def.heightCells = static_cast<int>(tilesetDef["__cHei"]);
		//def.identifier = copy_to_arena(tilesetDef["identifier"], arena);
		def.uid = static_cast<int>(tilesetDef["uid"]);
		def.relPath = copy_to_arena(tilesetDef["relPath"], arena);
		def.cellSize = static_cast<int>(tilesetDef["tileGridSize"]);
		def.spacing = static_cast<int>(tilesetDef["spacing"]);
		def.padding = static_cast<int>(tilesetDef["padding"]);
	}

	//
	// loading levels
	//
	auto docLevels = doc["levels"];
	nLevels = static_cast<int>(docLevels.count_elements());
	levels = static_cast<LdtkLevel*>(arena.push_zero(sizeof(LdtkLevel) * nLevels));
	i = 0;

	for (auto level : docLevels) {
		LdtkLevel& l = levels[i++];

		// NOTE(sand): same thing as the previous comment here
		l.identifier = copy_to_arena(level["identifier"], arena);
		l.iid = copy_to_arena(level["iid"], arena);

		l.pxWorldX = static_cast<int>(level["worldX"]);
		l.pxWorldY = static_cast<int>(level["worldY"]);
		l.worldDepth = static_cast<int>(level["worldDepth"]);
		l.pxWidth = static_cast<int>(level["pxWid"]);
		l.pxHeight = static_cast<int>(level["pxHei"]);
		
		auto layerInstances = level["layerInstances"].get_array();
		l.nLayers = static_cast<int>(layerInstances.count_elements());
		l.layers = static_cast<LdtkLayerInstance*>(arena.push(sizeof(LdtkLayerInstance) * l.nLayers));
		int j = 0;

		for (auto layerInst : layerInstances) {
			LdtkLayerInstance& li = l.layers[j++];
			li.identifier = copy_to_arena(layerInst["__identifier"], arena);
			
			std::string_view type = layerInst["__type"].get_string();
			if (0 == type.compare("Tiles"))
				li.type = LdtkLayerInstance::TILE;
			else if (0 == type.compare("IntGrid"))
				li.type = LdtkLayerInstance::INTGRID;
			else if (0 == type.compare("Entities"))
				li.type = LdtkLayerInstance::ENTITY;

			li.widthCells = static_cast<int>(layerInst["__cWid"]);
			li.heightCells = static_cast<int>(layerInst["__cHei"]);
			li.cellSize = static_cast<int>(layerInst["__gridSize"]);
			li.opacity = static_cast<float>(layerInst["__opacity"]);
			
			switch (li.type) {
			case LdtkLayerInstance::TILE: {
				// set tileset
				li.gridTile.tileset = nullptr;
				int tilesetUid = static_cast<int>(layerInst["__tilesetDefUid"]);
				for (int a = 0; a < nTilesets; a++) {
					if (tilesetUid == tilesets[a].uid) {
						li.gridTile.tileset = &tilesets[a];
						break;
					}
				}

				// load gridTiles
				auto gridTilesArray = layerInst["gridTiles"].get_array();
				li.nData = gridTilesArray.count_elements();
				li.gridTile.data = static_cast<LdtkGridTileInstance*>(arena.push(sizeof(LdtkGridTileInstance) * li.nData));
				int k = 0;
				for (auto tile : gridTilesArray) {
					LdtkGridTileInstance& gti = li.gridTile.data[k++];
					gti.layerX = static_cast<int>(tile["px"].at(0));
					gti.layerY = static_cast<int>(tile["px"].at(1));
					gti.srcX = static_cast<int>(tile["src"].at(0));
					gti.srcY = static_cast<int>(tile["src"].at(1));
					gti.flip = static_cast<int>(tile["f"]);
					gti.id = static_cast<int>(tile["t"]);
					gti.alpha = static_cast<float>(tile["a"]);
				}
			} break;
			case LdtkLayerInstance::INTGRID: {
				auto intGridArray = layerInst["intGridCsv"].get_array();
				li.nData = static_cast<int>(intGridArray.count_elements());
				li.intGridData = static_cast<int*>(arena.push(sizeof(int) * li.nData));
				int k = 0;
				for (auto element : intGridArray) {
					li.intGridData[k++] = static_cast<int>(element);
				}
			} break;
			case LdtkLayerInstance::ENTITY: {
				fprintf(stderr, "LdtkLoader entity instance layer not implemented yet!");
			} break;
			}
		}
	}
}

void GameWorld::cleanup() {
	arena.dealloc();
}

void GameWorld::load_assets(TextureAtlas& atlas) {
	mems::Arena& scratch = mems::get_scratch();
	size_t parentDirLen = strlen(parentDirPath);

	for (int i = 0; i < nTilesets; i++) {
		mems::ArenaScope forScope(scratch);

		LdtkTilesetDef& set = tilesets[i];
		
		char* tilesetPath = static_cast<char*>(scratch.push_data(parentDirPath, parentDirLen));
		scratch.push_data(set.relPath, strlen(set.relPath));
		scratch.push_zero(1);

		set.atlasIdx = atlas.add_to_atlas(nullptr, tilesetPath);
	}
}

void GameWorld::render(Gfx& gfx) {
	for (int i = 0; i < nLevels; i++) {
		LdtkLevel& level = levels[i];

		// skip rendering level if it is not visible
		if (!coll::rect_rect(
			static_cast<int>(gfx.cameraPos.x), static_cast<int>(gfx.cameraPos.y), Gfx::nesWidth, Gfx::nesHeight,
			level.pxWorldX, level.pxWorldY, level.pxWidth, level.pxHeight)
			) continue;

		for (int j = 0; j < level.nLayers; j++) {
			LdtkLayerInstance& li = level.layers[j];
			if (li.type == LdtkLayerInstance::TILE) {
				const uint32_t atlasIdx = li.gridTile.tileset->atlasIdx;
				const int tileSize = li.gridTile.tileset->cellSize;

				for (int k = 0; k < li.nData; k++) {
					const LdtkGridTileInstance& tile = li.gridTile.data[k];
					gfx.queue_sprite(tile.layerX, tile.layerY, atlasIdx, {tile.srcX, tile.srcY, tileSize, tileSize}, true);
				}
			}
		}
	}
}