#include "player.h"

#include "engine/game_context.h"
#include "engine/image_asset.h"
#include "engine/gfx.h"
#include "engine/input.h"

void Player::load(TextureAtlas& atlas) {
	x = 64;
	y = 64;

	animator.init(atlas.find_sprite("player"));
	sheet = atlas.subTextures[animator.spriteIdx].sheetData;
	assert(sheet);

	velocity = { 0.0f, 0.0f };
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
	if (y > (240 - currentRect.h)) {
		y = 240 - currentRect.h;
		velocity.y = -velocity.y;
	}
	
	// TODO(sand): See where player will move in the next frame, and test for collisions
	// if the GameContext has a GameWorld attached to it
	//if(ctx.world)
	
	x += velocity.x * ctx.delta;
	y += velocity.y * ctx.delta;

	animator.update(ctx.delta, *ctx.atlas->subTextures[animator.spriteIdx].sheetData);
}

void Player::render(Gfx& gfx) {
	gfx.queue_sprite(static_cast<int>(x), static_cast<int>(y), animator.spriteIdx, animator.current_frame(*sheet));
}
