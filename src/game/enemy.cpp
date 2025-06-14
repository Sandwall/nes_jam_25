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

	origin = { 18.0f, 18.0f };
	health = 20;

	for (int i = 0; i < NUM_PROJECTILES; i++) {
		projectiles[i].load(atlas);
	}
}

void Enemy::spawn(float x, float y) {
	pos = SDL_FPoint(x, y);
	active = true;
	detectedPlayer = false;

	for (int i = 0; i < NUM_PROJECTILES; i++) {
		projectiles[i].active = false;
	}
}

void Enemy::update(const GameContext& ctx) {
	movementTimer += ctx.delta;

	// update projectiles
	for (int i = 0; i < NUM_PROJECTILES; i++) {
		projectiles[i].update(ctx);
	}

	// check if player is within the enemy's detection range
	float dx = ctx.player->pos.x - pos.x;
	float dy = ctx.player->pos.y - pos.y;
	float distance = sqrtf((dx * dx) + (dy * dy));

	if (distance <= DETECTION_DISTANCE) {
		detectedPlayer = true;
	} else {
		detectedPlayer = false;
		fireTimer = 0.0f;
	}

	if (detectedPlayer) {
		// we've spotted the player so we want to start shooting
		fireTimer += ctx.delta;

		if (fireTimer >= FIRE_COOLDOWN) {
			for (int i = 0; i < NUM_PROJECTILES; i++) {
				if (!projectiles[i].active) {
					/* Check if enemy is currently moving left or right and update the booleans based on it for reversing
					the projectiles velocity */
					if (velocity.x >= 0.0f) projectiles[i].spawn(pos.x, pos.y, false);
					else if (velocity.x < 0.0f) projectiles[i].spawn(pos.x, pos.y, true);
					break;
				}
			}

			fireTimer = 0.0f;
		}
	}

	// Change the velocity of the enemy after certain time for testing
	if (movementTimer >= 0.0f && movementTimer <= 2.0f) {
		// Set the enemy's velocity to forward current velocity
		if (velocity.x != enemySpeedX) velocity.x = enemySpeedX;
	} else if (movementTimer > 2.0f && movementTimer <= 4.0f) {
		// Set the enemy's velocity to reverse current velocity
		if (velocity.x != -enemySpeedX) velocity.x = -enemySpeedX;
	} else if (movementTimer > 4.0f)
		movementTimer = 0.0f;

	// apply velocity to position
	pos.x += velocity.x * ctx.delta;
	pos.y += velocity.y * ctx.delta;

	animator.update(ctx.delta, *sheet);
}

void Enemy::render(Gfx& gfx) {
	SDL_FColor enemyColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	
	if (detectedPlayer) {
		enemyColor.b = 0.5f;
		enemyColor.g = 0.5f;
		// by this point, enemyColor is equal to { 1.0f, 0.5f, 0.5f, 1.0f }
	}

	gfx.queue_sprite(static_cast<int>(pos.x - origin.x), static_cast<int>(pos.y - origin.y), animator.spriteIdx, animator.current_frame(*sheet), true, enemyColor);
	
	for (int i = 0; i < NUM_PROJECTILES; i++) {
		projectiles[i].render(gfx);
	}
}
