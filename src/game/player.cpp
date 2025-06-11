#include "player.h"

#include "engine/game_context.h"
#include "engine/image_asset.h"
#include "engine/gfx.h"
#include "engine/input.h"

#include "game/world.h"

void Player::load(const TextureAtlas& atlas) {
	animator.init(atlas.find_sprite("player"));
	sheet = atlas.subTextures[animator.spriteIdx].sheetData;
	assert(sheet);

	pos = { 64.0f, 64.0f };
	velocity = { 0.0f, 0.0f };

	if (sheet->frames) {
		collBoxSize = { static_cast<float>(sheet->frames[0].source.w), static_cast<float>(sheet->frames[0].source.h) };
		origin = { collBoxSize.x / 2.0f, collBoxSize.y };
	} else {
		origin = { 8.0f, 32.0f };
		collBoxSize = { 16.0f, 32.0f };
	}
}

void Player::update(const GameContext& ctx) {
	constexpr float hSpeed = 64.0f;
	constexpr float gravity = hSpeed * 2.0f;
	const SDL_FRect& currentRect = animator.current_framef(*sheet);

	if (ctx.input->right)
		velocity.x = hSpeed;
	else if (ctx.input->left)
		velocity.x = -hSpeed;
	else
		velocity.x = 0.0f;

	// apply gravity
	velocity.y += gravity * ctx.delta;
	
	// TODO(sand): See where player will move in the next frame, and test for collisions
	// if the GameContext has a GameWorld attached to it

	//if(ctx.world)
	
	pos.x += velocity.x * ctx.delta;
	pos.y += velocity.y * ctx.delta;

	animator.update(ctx.delta, *ctx.atlas->subTextures[animator.spriteIdx].sheetData);
}

void Player::render(Gfx& gfx) {
	const SDL_FRect cbox = get_cboxf();
	gfx.queue_sprite(static_cast<int>(cbox.x), static_cast<int>(cbox.y), animator.spriteIdx, animator.current_frame(*sheet));
}
