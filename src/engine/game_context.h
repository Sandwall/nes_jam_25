#pragma once

#include <stdint.h>

struct GameContext {
	float delta;
	// we can fit another 4-byte value here

	// NOTE(sand): I thought it might be a good idea for the gameobject logic to check this value and update
	// only if it is not true, however it makes more sense to keep this outside of the gameobject logic and
	// instead keep track of it as a part of the main game loop
	// bool loading;

	uint64_t targetFps;
	uint64_t target_ns() const;

	// engine details
	struct SDL_Window* window;
	const struct Gfx* gfx;
	struct Input* input;
	struct TextureAtlas* atlas;

	// game details
	struct GameWorld* world;
	struct Player* player;
	// TODO(sand): eventually we might have pointers to enemies/bosses here

	// NOTE(sand): simdjson complains if I try to forward declare simdjson::ondemand::parser (probably from the backend auto-select feature)
	// so we'll just keep it as a void pointer for now, and just let the implementation file (game_context.cpp) deal with it
	static void init();
	static void cleanup();
	static void* jsonParser;
};

#define GET_JSON_PARSER reinterpret_cast<simdjson::ondemand::parser*>(GameContext::jsonParser)
