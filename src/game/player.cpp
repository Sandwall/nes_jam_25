#include "player.h"

#include "engine/game_context.h"
#include "engine/image_asset.h"
#include "engine/gfx.h"
#include "engine/input.h"

#include "game/world.h"

#include <math.h>

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

void Player::update(const GameContext& ctx) {
	constexpr float hSpeed = 128.0f;
	constexpr float gravity = hSpeed * 4.0f;
	constexpr float jumpSpeed = hSpeed * 1.75f;

	if (ctx.input->right) {
		facingLeft = false;
		velocity.x = hSpeed;
	} else if (ctx.input->left) {
		facingLeft = true;
		velocity.x = -hSpeed;
	} else velocity.x = 0.0f;

	// apply gravity
	velocity.y += gravity * ctx.delta;

	if (isGrounded && ctx.input->a.clicked())
		velocity.y = -jumpSpeed;

	if (!isGrounded && velocity.y < 0.0f && !ctx.input->a) {
		velocity.y /= 4.0f;
	}

	move_with_collision(ctx);

	animator.update(ctx.delta, *ctx.atlas->subTextures[animator.spriteIdx].sheetData);
}

constexpr SDL_FColor FCOL_WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };

void Player::render(Gfx& gfx) {
	gfx.queue_sprite(static_cast<int>(pos.x - origin.x), static_cast<int>(pos.y - origin.y),
		animator.spriteIdx, animator.current_frame(*sheet), true, FCOL_WHITE, facingLeft);
	
	// const SDL_FRect cbox = get_cboxf();
	// if (isGrounded) gfx.queue_rect(cbox, true, SDL_FColor{ 0.0f, 0.0f, 1.0f, 1.0f });
}

/*

// idk if it's worth it, but this function is hopefully branchless
float line_overlap(float x0, float len0, float x1, float len1) {
	// this is the assumption, otherwise we'll have to normalize both ranges (see commented function below)
	assert(len0 >= 0);
	assert(len1 >= 0);

	float select = static_cast<float>(x0 < x1);

	// adding leftStart + leftLength
	// leftLength = len0 if x0 is on the left, or len1 if this is not true
	float leftEnd = fminf(x0, x1) + (select * len0) + ((1.0f - select) * len1);

	// doing rightStart - leftEnd
	return fmaxf(0.0f, leftEnd - fmaxf(x0, x1));
}
*/

// NOTE(sand): This isn't used for now, primarily we're using SDL_GetRectCollisionFloat instead
float line_overlap(float x0, float len0, float x1, float len1) {
	float a0 = fminf(x0, x0 + len0);
	float a1 = fmaxf(x0, x0 + len0);
	float b0 = fminf(x1, x1 + len1);
	float b1 = fmaxf(x1, x1 + len1);
	float start = fmaxf(a0, b0);
	float end = fminf(a1, b1);
	return fmaxf(end - start, 0.0f);
}

void Player::move_with_collision(const GameContext& ctx) {
	prevPos = pos;

	// NOTE(sand): I'm going to try an approach where we keep track of how far we've moved from the previous position
	// when we loop through the solid tiles, we'll keep track of how much they one-dimensionally overlap with the player cbox
	// we can then take the maximum of those overlaps and then snap the player to distanceTraveled - maxOverlap using -signTraveled
	// this hypothetically should work great as long as all tiles are static and prevPos is never inside of a solid tile
	// i imagine that if prevPos is inside of a solid tile then the player will oscillate back and forth rapidly
	int left, right, top, bottom;
	SDL_FRect localPlayerCollRect;
	float distanceTraveled, signTraveled;
	SDL_FRect tileRect, intersectRect;

	auto update_bounds = [&](const LdtkLevel& room, const LdtkLayerInstance& layer) {
		localPlayerCollRect = get_cboxf();
		localPlayerCollRect.x -= room.pxWorldX;
		localPlayerCollRect.y -= room.pxWorldY;

		left = tim::clamp(0, static_cast<int>(localPlayerCollRect.x) / layer.cellSize, layer.widthCells - 1);
		right = tim::clamp(0, static_cast<int>(localPlayerCollRect.x + localPlayerCollRect.w) / layer.cellSize, layer.widthCells - 1);
		top = tim::clamp(0, static_cast<int>(localPlayerCollRect.y) / layer.cellSize, layer.heightCells - 1);
		bottom = tim::clamp(0, static_cast<int>(localPlayerCollRect.y + localPlayerCollRect.h) / layer.cellSize, layer.heightCells - 1);
	};

	// apply x velocity to position
	distanceTraveled = velocity.x * ctx.delta;
	signTraveled = static_cast<float>(tim::sign(distanceTraveled));
	pos.x += distanceTraveled;

	//
	// detect and resolve horizontal collision
	//
	for (int i = 0; i < ctx.nPlayerRooms; i++) {
		// we know playerRooms only has rooms that the player is touching
		const LdtkLevel& room = *ctx.playerRooms[i];
		const LdtkLayerInstance& cLayer = room.layers[room.collisionLayerIdx];
		update_bounds(room, cLayer);
		tileRect.w = static_cast<float>(cLayer.cellSize);
		tileRect.h = static_cast<float>(cLayer.cellSize);
		
		float maxOverlap = 0; bool collisionDetected = false;
		for (int y = top; y <= bottom; y++) {
			for (int x = left; x <= right; x++) {
				if (1 == cLayer.intGridData[(y * cLayer.widthCells) + x]) {
					tileRect.x = static_cast<float>(x * cLayer.cellSize);
					tileRect.y = static_cast<float>(y * cLayer.cellSize);
					if (SDL_GetRectIntersectionFloat(&tileRect, &localPlayerCollRect, &intersectRect)) {
						collisionDetected = true;
						if (intersectRect.w > maxOverlap)
							maxOverlap = intersectRect.w;
					}
				}
			}
		}

		if (collisionDetected) {
			pos.x -= signTraveled * (maxOverlap + 0.01f);
			velocity.x = 0;
		}
	}

	// apply y velocity to position
	distanceTraveled = velocity.y * ctx.delta;
	signTraveled = static_cast<float>(tim::sign(distanceTraveled));
	pos.y += distanceTraveled;

	//
	// detect and resolve vertical collision
	//
	for (int i = 0; i < ctx.nPlayerRooms; i++) {
		// we know playerRooms only has rooms that the player is touching
		const LdtkLevel& room = *ctx.playerRooms[i];
		const LdtkLayerInstance& cLayer = room.layers[room.collisionLayerIdx];
		update_bounds(room, cLayer);
		tileRect.w = static_cast<float>(cLayer.cellSize);
		tileRect.h = static_cast<float>(cLayer.cellSize);

		float maxOverlap = 0; bool collisionDetected = false;
		for (int y = top; y <= bottom; y++) {
			for (int x = left; x <= right; x++) {
				if (1 == cLayer.intGridData[(y * cLayer.widthCells) + x]) {
					tileRect.x = static_cast<float>(x * cLayer.cellSize);
					tileRect.y = static_cast<float>(y * cLayer.cellSize);
					if (SDL_GetRectIntersectionFloat(&tileRect, &localPlayerCollRect, &intersectRect)) {
						collisionDetected = true;
						isGrounded = distanceTraveled > 0.0;
						if (intersectRect.h > maxOverlap)
							maxOverlap = intersectRect.h;
					}
				}
			}
		}

		if (collisionDetected) {
			pos.y -= signTraveled * (maxOverlap + 0.01f);
			velocity.y = 0;
		} else {
			isGrounded = false;
		}
	}
}