#pragma once

#include "gameobject.h"
#include "engine/image_asset.h"

struct Player : GameObject {
	SpriteAnimator animator;
	SubTexture subtex;
	SpriteSheet* sheet; // loaded in by the atlas

	void load(TextureAtlas& atlas);
	void update(const GameContext& ctx) override;
	void render(Gfx& gfx) override;
};