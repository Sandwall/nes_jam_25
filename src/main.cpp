#include <SDL3/SDL.h>

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

#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <tinydef.hpp>
#include <mems.hpp>
#include <vector>
#include <string>

constexpr int defWidth = Gfx::nesWidth * 4, defHeight = Gfx::nesHeight * 4;
int windowWidth = defWidth, windowHeight = defHeight;    // these are here if we ever want it for some reason

Input input;
Gfx gfx;
TextureAtlas atlas;
GameWorld world;

Player player;
std::vector<Enemy> enemies;

GameContext game = {
	.delta = 1.0f / 60.0f,
	.targetFps = 60,
	.gfx = &gfx,
	.input = &input,
	.atlas = &atlas,
	.world = &world,
	.player = &player,
};

int init();
void cleanup();
int main_loop();

int main(int argc, char** argv) {
	if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD))
		return -1;

	if (-1 == init())
		return -1;

	int returnVal = main_loop();
	
	cleanup();
	SDL_Quit();
}

void update_camera();
void update_process_rooms();

int init() {
	mems::init();
	GameContext::init();

	// create window and init graphics
	game.window = SDL_CreateWindow("NES Game!", windowWidth, windowHeight, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
	SDL_SetWindowMinimumSize(game.window, Gfx::nesWidth, Gfx::nesHeight);
	if (!gfx.init(game.window))
		return -1;

	// load world (this happens before atlas creation because we need to prepare relPaths of the tilesets)
	world.init("./res/world1.ldtk");

	enemies.clear();
	enemies.resize(1);

	// create atlas and load all assets
	atlas.create(1024, 1024);
	gfx.fontIdx = atlas.add_to_atlas("font", "./res/font.png");
	atlas.add_to_atlas("player", "./res/mainChar/mage3.png", "./res/mainChar/mage3.json");
	atlas.add_to_atlas("enemy1", "./res/enemy1/enemy1.png", "./res/enemy1/enemy1.json");
	atlas.add_to_atlas("projectile1", "./res/fireball1/fireball1.png", "./res/fireball1/fireball1.json");
	world.load_assets(atlas);
	atlas.pack_atlas();
	gfx.upload_atlas(atlas);

	// load gameobjects from texture atlas
	// image assets are already loaded, the gameobjects simply just need to cache the indices of the assets they need
	player.load(atlas);

	// Load all the enemies using its texture atlas info
	for (int i = 0; i < enemies.size(); i++) enemies[i].load(atlas);

	// Spawn enemies at different locations
	enemies[0].spawn(50.0f, 100.0f);
	//enemies[1].spawn(150.0f, 100.0f);
	//enemies[2].spawn(300.0f, 100.0f);

	// Set the velocities of the different enemies or use for loop and set the velocities to the same value for every enemy
	enemies[0].velocity = { 5.0f, 0.0f };
	//enemies[1].velocity = { 5.0f, 0.0f };
	//enemies[2].velocity = { 5.0f, 0.0f };

	audio_init();

	SDL_ShowWindow(game.window);
	return 0;
}

void cleanup() {
	audio_close();
	gfx.cleanup();
	atlas.destroy();
	SDL_DestroyWindow(game.window);
	GameContext::cleanup();
	mems::close();
}

bool paused = false;
int main_loop() {
	while (true) {
		// timer start - this is meant for framelimiting
		Uint64 startFrame = SDL_GetTicksNS();

		// process events
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_EVENT_WINDOW_RESIZED:
				windowWidth = event.window.data1;
				windowHeight = event.window.data2;
			case SDL_EVENT_WINDOW_MOVED:
				game.delta = game.target_sec();
				startFrame = SDL_GetTicksNS();
				break;

			case SDL_EVENT_QUIT:
				return EXIT_SUCCESS;

			case SDL_EVENT_KEY_DOWN:
			case SDL_EVENT_KEY_UP:
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
			case SDL_EVENT_MOUSE_MOTION:
			case SDL_EVENT_GAMEPAD_AXIS_MOTION:
			case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			case SDL_EVENT_GAMEPAD_BUTTON_UP:
				input.handle_event(event);
				break;
			}
		}

		if (input.b.clicked()) {
			cleanup();
			init();
		}
		
		if (input.start.clicked())
			paused = !paused;

		//
		// update
		//
		if (!paused) {
			update_process_rooms();

			// update entities
			player.update(game);
			for (int i = 0; i < enemies.size(); i++) enemies[i].update(game); // Update all enemies

			update_camera();
		}

		input.end_frame();
		audio_tick();

		//
		// render
		//
		gfx.begin_frame();

		world.render(gfx, game);
		player.render(gfx);

		for (int i = 0; i < enemies.size(); i++) enemies[i].render(gfx); // Render all enemies using gfx

		if (paused) {
			constexpr int PAUSED_TEXT_X = (Gfx::nesWidth / 2) - (static_cast<int>(std::char_traits<char>::length("PAUSED") * 8) / 2);
			constexpr int PAUSED_TEXT_Y = (Gfx::nesHeight / 2) - 16;
			gfx.queue_text(PAUSED_TEXT_X, PAUSED_TEXT_Y, "PAUSED");
		}

		gfx.finish_frame();

		// end frame - framelimiting logic
		Uint64 elapsed = SDL_GetTicksNS() - startFrame;
		if (game.target_ns() > elapsed) {
			// keep in mind these values are unsigned, we do not want to have an underflow to UINT64_MAX
			SDL_DelayNS(game.target_ns() - elapsed);
		}

		// calculate delta time
		elapsed = SDL_GetTicksNS() - startFrame;
		game.delta = SDL_NS_TO_SECONDS(static_cast<float>(elapsed));
	}
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