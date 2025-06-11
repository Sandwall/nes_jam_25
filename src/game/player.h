#pragma once

#include "entity.h"
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

	void load(const struct TextureAtlas& atlas) override;
	void update(const struct GameContext& ctx) override;
	void render(struct Gfx& gfx) override;
};