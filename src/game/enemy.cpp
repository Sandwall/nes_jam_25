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

	for (int i = 0; i < NUM_PROJECTILES; i++) {
		projectiles[i].load(atlas);
	}
}

void Enemy::spawn(float x, float y) {
	pos = SDL_FPoint(x, y);
	active = true;

	for (int i = 0; i < NUM_PROJECTILES; i++) {
		projectiles[i].active = false;
	}
}

void Enemy::update(const GameContext& ctx) {
	movementTimer += ctx.delta;
	
	for (int i = 0; i < NUM_PROJECTILES; i++) {
		projectiles[i].update(ctx);
	}

	if (spottedPlayer) fireTimer += ctx.delta;
	else { if (fireTimer != 0.0f) fireTimer = 0.0f; }

	if (fireTimer >= FIRE_COOLDOWN && spottedPlayer) {
		for (int i = 0; i < NUM_PROJECTILES; i++) {
			if (!projectiles[i].active) 
			{
				/* Check if enemy is currently moving left or right and update the booleans based on it for reversing
				the projectiles velocity */
				if (velocity.x == enemySpeedX) projectiles[i].spawn(pos.x, pos.y, false);
				else if (velocity.x == -enemySpeedX) projectiles[i].spawn(pos.x, pos.y, true);
				break;
			}
		}

		fireTimer = 0.0f;
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
	gfx.queue_sprite(static_cast<int>(pos.x - origin.x), static_cast<int>(pos.y - origin.y), animator.spriteIdx, animator.current_frame(*sheet));
	
	for (int i = 0; i < NUM_PROJECTILES; i++) {
		projectiles[i].render(gfx);
	}
}

void Enemy::shoot_at_player(Player& player_)
{
	if (player_.pos.x >= pos.x && player_.pos.x <= pos.x + playerDistanceX && velocity.x == enemySpeedX &&
		player_.pos.y >= pos.y && player_.pos.y <= pos.y + playerDistanceY || // Enemy moving forward and below player
		player_.pos.x >= pos.x && player_.pos.x <= pos.x + playerDistanceX && velocity.x == enemySpeedX &&
		player_.pos.y <= pos.y && player_.pos.y >= pos.y - playerDistanceY || // Enemy moving forward and above player
		player_.pos.x <= pos.x && player_.pos.x >= pos.x - playerDistanceX && velocity.x == -enemySpeedX &&
		player_.pos.y <= pos.y && player_.pos.y >= pos.y - playerDistanceY || // Enemy moving backward and above
		player_.pos.x <= pos.x && player_.pos.x >= pos.x - playerDistanceX && velocity.x == -enemySpeedX &&
		player_.pos.y >= pos.y && player_.pos.y <= pos.y + playerDistanceY) // Enemy moving backward and below player
	{
		spottedPlayer = true;
	}

	else
	{
		spottedPlayer = false;
	}

	//printf("Player position x: %f, enemy position x: %f\n", player_.pos.x, pos.x);
	//printf("Player position y: %f, enemy position y: %f\n", player_.pos.y, pos.y);
}
