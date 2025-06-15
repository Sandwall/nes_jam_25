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
	health = MAX_HEALTH;

	for (int i = 0; i < NUM_PROJECTILES; i++) {
		projectiles[i].load(atlas);
	}

	if (sheet->frames) {
		collBoxSize = { static_cast<float>(sheet->frames[0].source.w), static_cast<float>(sheet->frames[0].source.h) };
		origin = { collBoxSize.x / 2.0f, collBoxSize.y / 2.0f };
		collBoxSize.y *= 3.0f / 4.0f;
	}
	else {
		collBoxSize = { 16.0f, 24.0f };
	}
}

void Enemy::spawn(float x, float y, float vx, float vy) {
	pos = { x, y };
	velocity = { vx, vy };
	active = true;
	detectedPlayer = false;

	for (int i = 0; i < NUM_PROJECTILES; i++) {
		projectiles[i].active = false;
	}
}

void Enemy::update(GameContext& ctx) {
	if (!active) return;

	movementTimer += ctx.delta;

	// update projectiles
	for (int i = 0; i < NUM_PROJECTILES; i++) {
		projectiles[i].update(ctx);

		if (projectiles[i].active) {
			SDL_FRect projectile = projectiles[i].get_cboxf();
			SDL_FRect player = ctx.player->get_cboxf();

			// Check if enemy's projectile hit the player
			if (SDL_HasRectIntersectionFloat(&projectile, &player)) {
				ctx.player->health -= 1;
				projectiles[i].active = false;
				//printf("Player health: %i\n", ctx.player->health);
			}
		}
	}

	if (ctx.enemies->health <= 0 && ctx.enemies->active) ctx.enemies->active = false;

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
					if (velocity.x >= 0.0f && ctx.player->pos.x >= pos.x)
						projectiles[i].spawn(pos.x, pos.y, 100.0f, 0.0f);
					else if (velocity.x < 0.0f && ctx.player->pos.x < pos.x)
						projectiles[i].spawn(pos.x, pos.y, -100.0f, 0.0f);

					break;
				}
			}

			fireTimer = 0.0f;
		}
	}

	if (movementTimer <= 2.0f) {
		// Set the enemy's velocity to forward current velocity
		if (velocity.x != enemySpeedX) velocity.x = enemySpeedX;
	} else if (movementTimer <= 4.0f) {
		// Set the enemy's velocity to reverse current velocity
		if (velocity.x != -enemySpeedX) velocity.x = -enemySpeedX;
	} else
		movementTimer = 0.0f;

	// apply velocity to position
	pos.x += velocity.x * ctx.delta;
	pos.y += velocity.y * ctx.delta;

	animator.update(ctx.delta, *sheet);
}

void Enemy::render(Gfx& gfx) {
	if (!active) return;

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
