#include "world.h"

#include "engine/game_context.h"
#include "engine/gfx.h"

#include <SDL3/SDL_filesystem.h>

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

void GameWorld::init(const char* path, GameContext& ctx) {
	arena.alloc();

	padded_string jsonString = padded_string::load(path);
	simdjson::ondemand::document doc = GET_JSON_PARSER->iterate(jsonString);

	// get parent directory path from filepath
	// TODO(sand): implement this, so that the parent directory path can be used to append a relPath to for tileset asset loading
	{
		mems::ArenaScope scope(arena);
		int pathLen = strlen(path);
		char* mutPath = static_cast<char*>(arena.push(pathLen));
		memcpy(mutPath, path, pathLen);

		for (int i = pathLen - 1; i >= 0; i--) {
			
		}
	}

	//
	// loading definitions
	// we only care about tilesets (see https://ldtk.io/json/#ldtk-DefinitionsJson)
	auto tilesetDefinitions = doc["defs"]["tilesets"].get_array();
	nTilesets = tilesetDefinitions.count_elements();
	tilesets = static_cast<LdtkTilesetDef*>(arena.push_zero(sizeof(LdtkTilesetDef) * nTilesets));
	int i = 0;

#define LOAD_STRING()

	for (auto tilesetDef : tilesetDefinitions) {
		LdtkTilesetDef& def = tilesets[i++];

		// NOTE(sand): take care to read these in the same order they are in the json document
		def.widthCells =  tilesetDef["__cWid"];
		def.heightCells = tilesetDef["__cHei"];
		def.identifier = copy_to_arena(tilesetDef["identifier"], arena);
		def.uid = tilesetDef["uid"];
		def.relPath = copy_to_arena(tilesetDef["relPath"], arena);
		def.cellSize = tilesetDef["tileGridSize"];
		def.spacing = tilesetDef["spacing"];
		def.padding = tilesetDef["padding"];
	}

	//
	// loading levels
	//
	auto docLevels = doc["levels"];
	nLevels = docLevels.count_elements();
	levels = static_cast<LdtkLevel*>(arena.push_zero(sizeof(LdtkLevel) * nLevels));
	i = 0;

	for (auto level : docLevels) {
		LdtkLevel& l = levels[i++];

		// NOTE(sand): same thing as the previous comment here
		l.identifier = copy_to_arena(level["identifier"], arena);
		l.iid = copy_to_arena(level["iid"], arena);

		l.pxWorldX = level["worldX"];
		l.pxWorldY = level["worldY"];
		l.worldDepth = level["worldDepth"];
		l.pxWidth = level["pxWid"];
		l.pxHeight = level["pxHei"];
		
		auto layerInstances = level["layerInstances"].get_array();
		l.nLayers = layerInstances.count_elements();
		int j = 0;
		l.layers = static_cast<LdtkLayerInstance*>(arena.push(sizeof(LdtkLayerInstance) * l.nLayers));

		for (auto layerInst : layerInstances) {
			LdtkLayerInstance& li = l.layers[j++];
			li.identifier = copy_to_arena(layerInst["__identifier"], arena);
			
			std::string_view type = layerInst["__type"];
			if (0 == type.compare("Tiles"))
				li.type = LdtkLayerInstance::TILE;
			else if (0 == type.compare("IntGrid"))
				li.type = LdtkLayerInstance::INTGRID;
			else if (0 == type.compare("Entities"))
				li.type = LdtkLayerInstance::ENTITY;

			li.widthCells = layerInst["__cWid"];
			li.heightCells = layerInst["__cHei"];
			li.cellSize = layerInst["__gridSize"];
			li.opacity = static_cast<float>(layerInst["__opacity"]);
			
			switch (li.type) {
			case LdtkLayerInstance::TILE: {
				// set tileset
				li.gridTile.tileset = nullptr;
				int tilesetUid = layerInst["__tilesetDefUid"];
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
					gti.layerX = tile["px"].at(0);
					gti.layerY = tile["px"].at(1);
					gti.srcX = tile["src"].at(0);
					gti.srcY = tile["src"].at(1);
					gti.flip = tile["f"];
					gti.id = tile["t"];
					gti.alpha = static_cast<float>(tile["a"]);
				}
			} break;
			case LdtkLayerInstance::INTGRID: {
				auto intGridArray = layerInst["intGridCsv"];
				li.nData = intGridArray.count_elements();
				li.intGridData = static_cast<int*>(arena.push(sizeof(int) * li.nData));
				int k = 0;
				for (int element : intGridArray) {
					li.intGridData[k++] = element;
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
	const char* basePath = SDL_GetBasePath();
	for (int i = 0; i < nTilesets; i++) {
		LdtkTilesetDef& set = tilesets[i];
		set.relPath
	}
}

void GameWorld::render(Gfx& gfx) {
}