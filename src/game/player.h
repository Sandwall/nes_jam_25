#pragma once

#include "gameobject.h"
#include "engine/image_asset.h"

#include <tinydef.hpp>

struct Player : GameObject {
	enum State {
		Idle,
		Walk,
		Air,
		Death,
		Max
	};

	tds::StateMachine<Max> state;
	SpriteAnimator animator;
	SubTexture subtex;
	SpriteSheet* sheet; // loaded in by the atlas

	void load(TextureAtlas& atlas);
	void update(const GameContext& ctx) override;
	void render(Gfx& gfx) override;
};