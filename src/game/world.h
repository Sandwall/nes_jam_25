#pragma once

#include <SDL3/SDL_rect.h>
#include <mems.hpp>

// Based on https://ldtk.io/json/#ldtk-DefinitionsJson
// It's implied that because of the redundant data stored for each entity, layer, and intgrid instance prefixed with "__"
// we won't really need to import LDtk world definitions when importing the world. However, these two structs are
// not fully covered by that "redundant" data, so we'll have to import those definitions at least...

//
// Definitions
//

// NOTE(sand): We probably won't implement LDtk enums, probs just gonna hardcode this type of thing since it's just a jam...
struct LdtkEnumDef {
	const char* externalRelPath;
	int iconTilesetUid;
	const char* identifier;
	int uid;

	int nEnums;
	struct Pair {
		const char* tag;
		int id;
	};
};

struct LdtkTilesetDef {
	const char* relPath;
	const char* identifier;    // this will be used for SubTexture retrieval, so it must match the SubTexture key
	int uid;                   // this is used to reference the tileset by LdtkLayerInstances
	
	// tileGridSize, __cWid, __cHei
	int cellSize, widthCells, heightCells;

	int padding;    // distance from image borders
	int spacing;    // distance between tiles

	//int pxWidth, pxHeight;     // dimension in pixels (can get this from dimension in cells * cell size)
};

//
// Instances
//

struct LdtkGridTileInstance {
	int layerX, layerY;
	int srcX, srcY;

	float alpha;
	int flip : 4;
	int id : 28;
};

struct LdtkEntityInstance {
	const char* identifier;
	int uid;

	int worldX, worldY;
	int width, height;
};

struct LdtkLayerInstance {
	int widthCells, heightCells, cellSize;    // __cWid, __cHei, __gridSize
	const char* identifier;    // __identifier
	float opacity;    // __opacity

	// int __pxTotalOffsetX, __pxTotalOffsetY
	// int pxOffsetX, pxOffsetY;
	// We're not going to be keeping track of offsets, as there are multiple fields that contribute to offset
	// In addition to this, for the purposes of the game we want tiles to be snapped to the 8x8 grid

	bool visible;
	bool parallaxScaling;
	float parallaxFactorX, parallaxFactorY;

	// NOTE(sand): Following the standard "tagged union" approach here
	enum Type {
		INTGRID,
		ENTITY,
		TILE,
	} type;

	int nData;
	union {
		// loaded from intGridCsv
		int* intGridData;

		struct {
			// we use the tileset uid to find this
			LdtkTilesetDef* tileset;

			// loaded from gridTiles
			LdtkGridTileInstance* data;
		} gridTile;

		// loaded from entityInstances
		LdtkEntityInstance* entityData;
	};
};

struct LdtkLevel {
	const char* identifier;
	const char* iid;

	// we can use this information to cull levels that the player can't see
	int pxWorldX, pxWorldY;    // position of level in world in pixels
	int pxWidth, pxHeight;     // size of level in pixels
	int worldDepth;            // > 0 means above, < 0 means below

	int nLayers;
	LdtkLayerInstance* layers;

	SDL_Rect get_bounding_box();
};

struct Gfx;
struct GameContext;
struct TextureAtlas;

struct GameWorld {
	int nTilesets;
	int nLevels;
	LdtkTilesetDef* tilesets;
	LdtkLevel* levels;

	const char* parentDirPath;

	// allocates and deallocates memory for the world
	// also loads levels from ldtk file
	void init(const char* path, GameContext& ctx);
	void cleanup();

	// call this after init, before packing atlas
	void load_assets(TextureAtlas& atlas);

	bool shouldDrawTiles = true;
	bool shouldDrawInt = true;
	void render(Gfx& gfx);
private:
	mems::Arena arena;
};