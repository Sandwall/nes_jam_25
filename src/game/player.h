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

	tds::StateMachine<Max> state;
	bool isGrounded = false;
	bool facingLeft = false;    // sprite faces right, so facingLeft = flipH

	void move_with_collision(const struct GameContext& ctx);

	void load(const struct TextureAtlas& atlas) override;
	void update(const struct GameContext& ctx) override;
	void render(struct Gfx& gfx) override;

	int health;
};