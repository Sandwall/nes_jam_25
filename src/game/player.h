#pragma once

#include "entity.h"
#include "engine/image_asset.h"

#include <tinydef.hpp>

struct Player : Entity {
	enum State {
		Idle,
		Walk,
		Air,
		Death,
		Max
	};

	tds::T2<float> velocity;

	tds::StateMachine<Max> state;

	void load(TextureAtlas& atlas) override;
	void update(const GameContext& ctx) override;
	void render(Gfx& gfx) override;
};