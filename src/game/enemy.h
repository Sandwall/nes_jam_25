#ifndef ENEMY_H
#define ENEMY_H

#include "entity.h"
#include "projectile.h"
#include <tinydef.hpp>
#include <vector>
#include <memory>

struct Enemy : public Entity {
	float enemyTime;

	void spawn(float x, float y);

	void load(const struct TextureAtlas& atlas) override;
	void update(const struct GameContext& ctx) override;
	void render(struct Gfx& gfx) override;

private:
	std::shared_ptr<Projectile> projectileObj;
	std::vector<std::shared_ptr<Projectile>> projectiles;

	float fireTime;
	const float fireCooldown = 0.2f;
	float fireLifetime;
};

#endif