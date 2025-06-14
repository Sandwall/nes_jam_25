#include "projectile.h"

#include "engine/game_context.h"
#include "engine/image_asset.h"
#include "engine/gfx.h"
#include "engine/input.h"
#include "game/player.h"

#include "game/world.h"

void Projectile::spawn(float x, float y, bool reverseProjectile_) {
	pos = SDL_FPoint(x, y);
	active = true;
	lifeTimer = 0.0f;
	velocity = { 100.0f, 0.0f };
	origin = { 8.0f, 8.0f };

	reverseProjectile = reverseProjectile_;
}

void Projectile::load(const struct TextureAtlas& atlas) {
	animator.init(atlas.find_sprite("projectile1"));
	sheet = atlas.subTextures[animator.spriteIdx].sheetData;
	assert(sheet);

	velocity = { 0.0f, 0.0f };
	lifeTimer = LIFETIME + 1.0f;
	active = false;
}

void Projectile::update(const struct GameContext& ctx) {
	if (active) {
		lifeTimer += ctx.delta;

		if (lifeTimer >= LIFETIME) {
			active = false;
			return;
		}

		SDL_FRect playerRect = ctx.player->get_cboxf();
		SDL_FRect projectileRect = get_cboxf();

		if (SDL_HasRectIntersectionFloat(&playerRect, &projectileRect))
		{
			ctx.player->health -= 1;
			active = false;
			//printf("HERE\n");
		}

		if (!reverseProjectile) velocity.x = 100.0f; // Move to right for testing
		else if (reverseProjectile) velocity.x = -100.0f; // Move to left for testing

		// apply velocity to position
		pos.x += velocity.x * ctx.delta;
		pos.y += velocity.y * ctx.delta;
		animator.update(ctx.delta, *sheet);
	}
}

void Projectile::render(struct Gfx& gfx) {
	if (active) {
		gfx.queue_sprite(static_cast<int>(pos.x - origin.x), static_cast<int>(pos.y - origin.y), animator.spriteIdx, animator.current_frame(*sheet));
	}

	if (!active) {
		int i = 0;
	}
}