#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "entity.h"
#include <tinydef.hpp>

struct Projectile : public Entity
{
	void spawn(float x, float y);

	void load(const struct TextureAtlas& atlas) override;
	void update(const struct GameContext& ctx) override;
	void render(struct Gfx& gfx) override;
};

#endif