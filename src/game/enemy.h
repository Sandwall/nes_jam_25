#ifndef ENEMY_H
#define ENEMY_H

#include "entity.h"
#include <tinydef.hpp>

struct Enemy : public Entity
{
	enum State {
		Idle,
		Walk,
		Air,
		Death,
		Max
	};

	tds::T2<float> velocity;
	tds::StateMachine<Max> state;
	float enemyTime;

	void load(const struct TextureAtlas& atlas) override;
	void spawn(const float& x_, const float& y_);
	void set_velocity(const float& speedX_, const float& speedY_);
	void update(const struct GameContext& ctx) override;
	void render(struct Gfx& gfx) override;
};

#endif