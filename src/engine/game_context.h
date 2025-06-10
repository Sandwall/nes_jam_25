#pragma once

#include <stdint.h>

struct SDL_Window;

struct TextureAtlas;
struct Input;
struct GameWorld;

struct GameContext {
	float delta;
	// need another 4byte val here

	uint64_t targetFps;

	uint64_t target_ns() const {
		if (targetFps == 0) return 0;
		return 1000000000LL / targetFps;
	}

	SDL_Window* window;
	Input* input;
	TextureAtlas* atlas;
	GameWorld* world;

	// NOTE(sand): simdjson complains if I try to forward declare simdjson::ondemand::parser (probably from the backend auto-select feature)
	// so we'll just keep it as a void pointer for now, and just let the implementation file (game_context.cpp) deal with it
	static void init();
	static void cleanup();
	static void* jsonParser;
};

#define GET_JSON_PARSER reinterpret_cast<simdjson::ondemand::parser*>(GameContext::jsonParser)
