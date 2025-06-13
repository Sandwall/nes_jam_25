#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "entity.h"
#include <tinydef.hpp>

struct Projectile : public Entity {
	void spawn(float x, float y);
	void load(const struct TextureAtlas& atlas) override;
	void update(const struct GameContext& ctx) override;
	void render(struct Gfx& gfx) override;
	static constexpr float LIFETIME = 2.0f;     // our projectile will live for 1 second
	float lifeTimer = 0.0f;
};

#endif