#pragma once

#include "engine/image_asset.h"

struct GameContext;
struct Gfx;

struct Entity {
	bool active = true;
	float x = 0, y = 0;

	SpriteAnimator animator = {};
	SpriteSheet* sheet = nullptr; // loaded in by the atlas

	virtual void load(TextureAtlas& atlas) = 0;
	virtual void update(const GameContext& ctx) = 0;
	virtual void render(Gfx& gfx) = 0;
};