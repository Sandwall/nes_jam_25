#include "projectile.h"

#include "engine/game_context.h"
#include "engine/image_asset.h"
#include "engine/gfx.h"
#include "engine/input.h"

#include "game/world.h"

void Projectile::spawn(float x, float y)
{
	pos = SDL_FPoint(x, y);
	active = true;
}

void Projectile::load(const struct TextureAtlas& atlas)
{
	animator.init(atlas.find_sprite("projectile1"));
	sheet = atlas.subTextures[animator.spriteIdx].sheetData;
	assert(sheet);
}

void Projectile::update(const struct GameContext& ctx)
{
	velocity.x += -10.0f; // Move to left for testing

	// apply velocity to position
	pos.x += velocity.x * ctx.delta;
	pos.y += velocity.y * ctx.delta;

	animator.update(ctx.delta, *ctx.atlas->subTextures[animator.spriteIdx].sheetData);
}

void Projectile::render(struct Gfx& gfx)
{
	gfx.queue_sprite(static_cast<int>(pos.x - origin.x), static_cast<int>(pos.y - origin.y), animator.spriteIdx, animator.current_frame(*sheet));
}