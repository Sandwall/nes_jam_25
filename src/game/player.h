#pragma once

#include "entity.h"
#include "projectile.h"

constexpr int NUM_ATTACKS = 6;

struct Player : Entity {
	enum AState {
		AS_IDLE = 0,
		AS_ATTACK,
		AS_DAMAGE,
		AS_DEATH,
	};

	bool isGrounded = false;
	bool facingLeft = false;    // sprite faces right, so facingLeft = flipH

	static constexpr int MAX_HEALTH = 5;
	int health = MAX_HEALTH;

	// timer that tracks how long the player was in the air
	static constexpr float COYOTE_LIMIT = 0.1f;
	float coyoteTimer;

	// timer that queues A-button presses, so that player can press it right before they hit the ground
	static constexpr float JUMPBUF_LIMIT = 0.2f;
	float jumpBuffer;

	void move_with_collision(const struct GameContext& ctx);

	void load(const struct TextureAtlas& atlas) override;
	void update(struct GameContext& ctx) override;
	void render(struct Gfx& gfx) override;

private:

	Projectile projectiles[NUM_ATTACKS];

	static constexpr float FIRE_COOLDOWN = 0.4f;
	float fireTimer;

	static constexpr int maxHealth = 5;
};