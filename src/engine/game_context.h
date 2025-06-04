#pragma once

#include <stdint.h>

struct SDL_Window;

struct Gfx;
struct TextureAtlas;
struct Input;

struct GameContext {
	float delta;
	// need another 4byte val here

	uint64_t targetFps;

	uint64_t target_ns() {
		if (targetFps == 0) return 0;
		return 1000000000LL / targetFps;
	}

	SDL_Window* window;
	Gfx* gfx;
	TextureAtlas* textures;

	Input* input;

	
};