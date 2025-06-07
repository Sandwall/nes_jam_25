#include "player.h"

#include "engine/game_context.h"
#include "engine/image_asset.h"
#include "engine/gfx.h"

void Player::load(TextureAtlas& atlas) {
	animator.spriteIdx = atlas.add_to_atlas("./res/mainChar/mage3.png", "./res/mainChar/mage3.json");
	animator.init();

	sheet = atlas.subTextures[animator.spriteIdx].sheetData;
}

void Player::update(const GameContext& ctx) {
	animator.update(ctx.delta, *ctx.atlas->subTextures[animator.spriteIdx].sheetData);
}

void Player::render(Gfx& gfx) {
	gfx.queue_sprite(x, y, animator.spriteIdx, animator.current_frame(*sheet));
}
