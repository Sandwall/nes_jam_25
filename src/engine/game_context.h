#pragma once

// NOTE(sand): Try to ensure this header does not get included in any other engine code

#include <stdint.h>

struct GameContext {
	float delta;
	uint32_t points;

	// we can fit another 4-byte value here

	// NOTE(sand): I thought it might be a good idea for the gameobject logic to check this value and update
	// only if it is not true, however it makes more sense to keep this outside of the gameobject logic and
	// instead keep track of it as a part of the main game loop
	// bool loading;

	uint64_t targetFps;
	float target_sec() const;
	uint64_t target_ns() const;

	// engine details
	struct SDL_Window* window;
	const struct Gfx* gfx;
	struct Input* input;
	struct TextureAtlas* atlas;

	// game details (eventually we might have pointers to enemies/bosses here)
	int nEnemies;
	struct GameWorld* world;
	struct Player* player;
	struct Enemy* enemies;

	// at the start of every frame, the game looks at which rooms the camera can see and which room the player
	// is in and updates these with at max 4 of those rooms. It's fine to do this at the start of the frame
	// because we should have designed the level structure such that the next frame's processRooms must have
	// at least one of the previous frames processRooms. 
	static constexpr int NUM_PROCESS_ROOMS = 4;
	int nProcessRooms, nPlayerRooms;
	struct LdtkLevel* processRooms[NUM_PROCESS_ROOMS];
	struct LdtkLevel* playerRooms[NUM_PROCESS_ROOMS];


	// NOTE(sand): simdjson complains if I try to forward declare simdjson::ondemand::parser (probably from the backend auto-select feature)
	// so we'll just keep it as a void pointer for now, and just let the implementation file (game_context.cpp) deal with it
	static void init();
	static void cleanup();
	static void* jsonParser;
};

#define GET_JSON_PARSER reinterpret_cast<simdjson::ondemand::parser*>(GameContext::jsonParser)
