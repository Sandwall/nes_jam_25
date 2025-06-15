#ifndef ENEMY_H
#define ENEMY_H

#include "entity.h"
#include "projectile.h"
#include "player.h"
#include <tinydef.hpp>

constexpr int NUM_PROJECTILES = 4;

struct Enemy : public Entity {
	float movementTimer;

	void spawn(float x, float y, float vx, float vy);

	void load(const struct TextureAtlas& atlas) override;
	void update(struct GameContext& ctx) override;
	void render(struct Gfx& gfx) override;
	
	int health = MAX_HEALTH;

private:
	Projectile projectiles[NUM_PROJECTILES];

	static constexpr float FIRE_COOLDOWN = 0.4f;
	float fireTimer;

	static constexpr float DETECTION_DISTANCE = 64.0f;
	bool detectedPlayer;

	static constexpr int MAX_HEALTH = 5.0f;
};

#endif