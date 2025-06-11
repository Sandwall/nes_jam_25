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
		collBoxSize.y *= 3.0f / 4.0f;
	} else {
		collBoxSize = { 16.0f, 24.0f };
		origin = { 8.0f, 32.0f };
	}
}

// NOTE(sand): this was for debugging purposes, to draw squares where the player would intersect with the collision intgrid
//int nCheckedPos;
//SDL_FPoint checkedPos[16];

void Player::update(const GameContext& ctx) {
	constexpr float hSpeed = 64.0f;
	constexpr float gravity = hSpeed * 2.0f;

	if (ctx.input->right)
		velocity.x = hSpeed;
	else if (ctx.input->left)
		velocity.x = -hSpeed;
	else
		velocity.x = 0.0f;

	if (ctx.input->down)
		velocity.y = hSpeed;
	else if (ctx.input->up)
		velocity.y = -hSpeed;
	else
		velocity.y = 0.0f;

	// apply gravity
	//velocity.y += gravity * ctx.delta;
	

	// apply velocity to position
	pos.x += velocity.x * ctx.delta;
	pos.y += velocity.y * ctx.delta;
	
	// detect and resolve collision
	SDL_FRect playerCollRect = get_cboxf();
	for (int i = 0; i < ctx.nProcessRooms; i++) {
		const LdtkLevel& room = *ctx.processRooms[i];
		SDL_FRect roomRect = room.get_bboxf();

		// only proceed for rooms that the player is touching
		if (!SDL_HasRectIntersectionFloat(&playerCollRect, &roomRect)) continue;
		
		SDL_FRect localCollRect = playerCollRect;
		localCollRect.x -= room.pxWorldX;
		localCollRect.y -= room.pxWorldY;

		const LdtkLayerInstance& layer = room.layers[room.collisionLayerIdx];
		int left = tim::max(0, static_cast<int>(localCollRect.x) / layer.cellSize);
		int right = tim::max(0, static_cast<int>(localCollRect.x + localCollRect.w) / layer.cellSize);
		int top = tim::max(0, static_cast<int>(localCollRect.y) / layer.cellSize);
		int bottom = tim::max(0, static_cast<int>(localCollRect.y + localCollRect.h) / layer.cellSize);

		bool coll = false;
		//nCheckedPos = 0;
		for (int x = left; x <= right; x++) {
			for (int y = top; y <= bottom; y++) {
				if (1 == layer.intGridData[(y * layer.widthCells) + x]) {
					// now perform collision response

					//checkedPos[nCheckedPos].x = (x * layer.cellSize);
					//checkedPos[nCheckedPos].y = (y * layer.cellSize);
					//nCheckedPos++;
				}
			}
		}
	}

	animator.update(ctx.delta, *ctx.atlas->subTextures[animator.spriteIdx].sheetData);
}

void Player::render(Gfx& gfx) {
	gfx.queue_sprite(static_cast<int>(pos.x - origin.x), static_cast<int>(pos.y - origin.y), animator.spriteIdx, animator.current_frame(*sheet));
	/*
	const SDL_FRect cbox = get_cboxf();
	gfx.queue_rect(cbox, true, SDL_FColor{ 0.0f, 0.0f, 1.0f, 1.0f });
	SDL_FRect rec{ 0.0f, 0.0f, 8.0f, 8.0f };
	for (int i = 0; i < nCheckedPos; i++) {
		rec.x = checkedPos[i].x;
		rec.y = checkedPos[i].y;
		gfx.queue_rect(rec, true, SDL_FColor{ 0.0f, 1.0f, 0.0f, 1.0f });
	}

	SDL_FPoint pt = { pos.x, pos.y };
	gfx.queue_point(pt, true, SDL_FColor{ 1.0f, 1.0f, 0.0f, 1.0f });
	pt = { pos.x - origin.x, pos.y - origin.y };
	gfx.queue_point(pt, true, SDL_FColor{ 1.0f, 1.0f, 0.0f, 1.0f });
	*/
}
