#include "enemy.h"

#include "engine/game_context.h"
#include "engine/image_asset.h"
#include "engine/gfx.h"
#include "engine/input.h"

#include "game/world.h"

constexpr float enemySpeedX{ 20.0f };

void Enemy::load(const TextureAtlas& atlas) {
	animator.init(atlas.find_sprite("enemy1"));
	sheet = atlas.subTextures[animator.spriteIdx].sheetData;
	assert(sheet);

	/*if (sheet->frames) {
		collBoxSize = { static_cast<float>(sheet->frames[0].source.w), static_cast<float>(sheet->frames[0].source.h) };
		origin = { collBoxSize.x / 2.0f, collBoxSize.y };
		collBoxSize.y *= 3.0f / 4.0f;
	}
	else {
		collBoxSize = { 16.0f, 24.0f };
		origin = { 8.0f, 32.0f };
	}*/
}

void Enemy::spawn(float x, float y) {
	pos = SDL_FPoint(x, y);
	active = true;
}

void Enemy::update(const GameContext& ctx) {
	enemyTime += ctx.delta;

	// Change the velocity of the enemy after certain time for testing
	if (enemyTime >= 0.0f && enemyTime <= 2.0f) {
		// Set the enemy's velocity to forward current velocity
		if (velocity.x != enemySpeedX) velocity.x = enemySpeedX;
	} else if (enemyTime > 2.0f && enemyTime <= 4.0f) {
		// Set the enemy's velocity to reverse current velocity
		if (velocity.x != -enemySpeedX) velocity.x = -enemySpeedX;
	} else if (enemyTime > 4.0f)
		enemyTime = 0.0f;

	// apply velocity to position
	pos.x += velocity.x * ctx.delta;
	pos.y += velocity.y * ctx.delta;

	animator.update(ctx.delta, *ctx.atlas->subTextures[animator.spriteIdx].sheetData);
}

void Enemy::render(Gfx& gfx) {
	gfx.queue_sprite(static_cast<int>(pos.x - origin.x), static_cast<int>(pos.y - origin.y), animator.spriteIdx, animator.current_frame(*sheet));
}
