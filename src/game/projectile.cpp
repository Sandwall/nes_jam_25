#include "projectile.h"

#include "engine/game_context.h"
#include "engine/image_asset.h"
#include "engine/gfx.h"
#include "engine/input.h"

#include "game/world.h"

void Projectile::spawn(float x, float y, float vx, float vy) {
	pos = SDL_FPoint(x, y);
	active = true;
	lifeTimer = 0.0f;
	velocity = { vx, vy };
	origin = { 8.0f, 8.0f };
}

void Projectile::load(const TextureAtlas& atlas) {
	animator.init(atlas.find_sprite("projectile1"));
	sheet = atlas.subTextures[animator.spriteIdx].sheetData;
	assert(sheet);

	lifeTimer = LIFETIME + 1.0f;
	active = false;
}

void Projectile::update(GameContext& ctx) {
	if (active) {
		lifeTimer += ctx.delta;

		if (lifeTimer >= LIFETIME) {
			active = false;
			return;
		}

		// apply velocity to position
		pos.x += velocity.x * ctx.delta;
		pos.y += velocity.y * ctx.delta;
		animator.update(ctx.delta, *sheet);
	}
}

void Projectile::render(Gfx& gfx) {
	if (active) {
		gfx.queue_sprite(
			static_cast<int>(pos.x - origin.x), static_cast<int>(pos.y - origin.y),
			animator.spriteIdx, animator.current_frame(*sheet), true, FCOL_WHITE,
			velocity.x < 0.0f);
	}
}