#ifndef ENEMY_H
#define ENEMY_H

#include "entity.h"
#include <tinydef.hpp>

struct Enemy : public Entity {
	float enemyTime;

	void spawn(float x, float y);

	void load(const struct TextureAtlas& atlas) override;
	void update(const struct GameContext& ctx) override;
	void render(struct Gfx& gfx) override;
};

#endif