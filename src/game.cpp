#include "game.h"

#define _USE_MATH_DEFINES
#include <math.h>
#define TINYDEF_EXPF expf
#define TINYDEF_EXP exp

#include "engine/input.h"
#include "engine/gfx.h"
#include "engine/game_context.h"
#include "engine/image_asset.h"
#include "engine/audio.h"

#include "game/player.h"
#include "game/enemy.h"
#include "game/world.h"

#include <vector>
#include <tinydef.hpp>

extern Input input;
extern Gfx gfx;
extern TextureAtlas atlas;
extern GameWorld world;
extern Player player;
extern std::vector<Enemy> enemies;
extern GameContext game;

void game_init() { 
	// load world (this happens before atlas creation because we need to prepare relPaths of the tilesets)
	world.init("./res/world1.ldtk");

	// create atlas and load all assets
	atlas.create(1024, 1024);
	gfx.fontIdx = atlas.add_to_atlas("font", "./res/font.png");
	atlas.add_to_atlas("player", "./res/mainChar/mage3.png", "./res/mainChar/mage3.json");
	atlas.add_to_atlas("enemy1", "./res/enemy1/enemy1.png", "./res/enemy1/enemy1.json");
	atlas.add_to_atlas("projectile1", "./res/fireball1/fireball1.png", "./res/fireball1/fireball1.json");
	world.load_assets(atlas);
	atlas.pack_atlas();
	gfx.upload_atlas(atlas);

	Enemy& enemy = enemies.emplace_back();

	// load gameobjects from texture atlas
	// image assets are already loaded, the gameobjects simply just need to cache the indices of the assets they need
	player.load(atlas);
	for (int i = 0; i < enemies.size(); i++) enemies[i].load(atlas);

	enemy.spawn(50.0f, 100.0f, 5.0f, 0.0f);
}

void update_process_rooms();
void update_camera();

void game_update() {
	game.nEnemies = static_cast<int>(enemies.size());
	game.enemies = enemies.data();

	update_process_rooms();

	// update entities
	player.update(game);
	for (int i = 0; i < enemies.size(); i++) enemies[i].update(game);

	update_camera();
}

void game_render() {
	world.render(gfx, game);
	player.render(gfx);

	for (int i = 0; i < enemies.size(); i++) enemies[i].render(gfx);
}

constexpr float CAM_SPEED = 7.5f;
void update_camera() {
	int targetX = static_cast<int>(player.pos.x) - (Gfx::nesWidth / 2);
	int targetY = static_cast<int>(player.pos.y) - (Gfx::nesHeight / 2);

	// find room that player overlaps with the most
	const SDL_FRect playerRect = player.get_cboxf();
	float maxArea = 0;
	const LdtkLevel* playerRoom = nullptr;
	for (int i = 0; i < game.nPlayerRooms; i++) {
		const LdtkLevel& room = *game.playerRooms[i];
		const SDL_FRect roomRect = room.get_bboxf();

		SDL_FRect intersection = { 0 };
		SDL_GetRectIntersectionFloat(&playerRect, &roomRect, &intersection);

		const float area = intersection.w * intersection.h;
		if (area > maxArea) {
			maxArea = area;
			playerRoom = &room;
		}
	}

	// if we even had a most overlapped room, then clamp target coordinates to that room
	if (playerRoom) {
		targetX = tim::clamp(targetX, playerRoom->pxWorldX, playerRoom->pxWidth - Gfx::nesWidth);
		targetY = tim::clamp(targetY, playerRoom->pxWorldY, playerRoom->pxHeight - Gfx::nesHeight);
	}

	gfx.cameraPos.x = tim::filerp32(gfx.cameraPos.x, static_cast<float>(targetX), CAM_SPEED, game.delta);
	gfx.cameraPos.y = tim::filerp32(gfx.cameraPos.y, static_cast<float>(targetY), CAM_SPEED, game.delta);
}

void update_process_rooms() {
	memset(game.processRooms, 0, sizeof(LdtkLevel*) * GameContext::NUM_PROCESS_ROOMS);
	memset(game.playerRooms, 0, sizeof(LdtkLevel*) * GameContext::NUM_PROCESS_ROOMS);
	game.nProcessRooms = 0;
	game.nPlayerRooms = 0;
	const SDL_FRect camBbox = gfx.cam_bboxf();
	const SDL_FRect playerCbox = player.get_cboxf();

	for (int i = 0; i < world.nLevels; i++) {
		LdtkLevel& level = world.levels[i];
		const SDL_FRect levelBbox = level.get_bboxf();

		if (SDL_HasRectIntersectionFloat(&playerCbox, &levelBbox) && game.nPlayerRooms < GameContext::NUM_PROCESS_ROOMS) {
			game.playerRooms[game.nPlayerRooms++] = &level;
		}

		if (SDL_HasRectIntersectionFloat(&camBbox, &levelBbox) && game.nProcessRooms < GameContext::NUM_PROCESS_ROOMS) {
			game.processRooms[game.nProcessRooms++] = &level;
		}
	}
}