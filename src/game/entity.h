#pragma once

#include "engine/image_asset.h"

#include <SDL3/SDL_rect.h>

struct Entity {
	bool active = true;

	SDL_FPoint prevPos;
	SDL_FPoint pos;
	SDL_FPoint origin;
	SDL_FPoint collBoxSize;

	SDL_FPoint velocity;

	SpriteAnimator animator = {};
	const SpriteSheet* sheet = nullptr; // loaded in by the atlas

	virtual void load(const TextureAtlas& atlas) = 0;
	virtual void update(struct GameContext& ctx) = 0;
	virtual void render(struct Gfx& gfx) = 0;

	bool is_in_room(const struct LdtkLevel* level);
	SDL_FRect get_cboxf();
};