#pragma once

#include "entity.h"

struct Player : Entity {

	bool isGrounded = false;
	bool facingLeft = false;    // sprite faces right, so facingLeft = flipH

	// timer that tracks how long the player was in the air
	float coyoteTimer;

	// timer that queues A-button presses, so that player can press it right before they hit the ground
	float jumpBuffer;

	void move_with_collision(const struct GameContext& ctx);

	void load(const struct TextureAtlas& atlas) override;
	void update(const struct GameContext& ctx) override;
	void render(struct Gfx& gfx) override;
};