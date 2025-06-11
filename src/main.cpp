#include <SDL3/SDL.h>

#include "engine/input.h"
#include "engine/gfx.h"
#include "engine/game_context.h"
#include "engine/image_asset.h"

#include "game/player.h"
#include "game/world.h"

#include <stdio.h>
#include <math.h>

#include <tinydef.hpp>
#include <mems.hpp>

constexpr int defWidth = Gfx::nesWidth * 4, defHeight = Gfx::nesHeight * 4;

Input input;
Gfx gfx;
TextureAtlas atlas;
GameWorld world;

Player player;

GameContext game = {
	.delta = 1.0f / 60.0f,
	.targetFps = 60,
	.gfx = &gfx,
	.input = &input,
	.atlas = &atlas,
	.world = &world,
	.player = &player,
};

int main_loop();

int main(int argc, char** argv) {
	if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD))
		return -1;

	mems::init();
	GameContext::init();

	// create window and init graphics
	game.window = SDL_CreateWindow("NES Game!", defWidth, defHeight, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
	SDL_SetWindowMinimumSize(game.window, Gfx::nesWidth * 2, Gfx::nesHeight * 2);
	if (!gfx.init(game.window))
		return -1;

	// create atlas and load all assets
	atlas.create(1024, 1024);
	gfx.fontIdx = atlas.add_to_atlas("font", "./res/font.png");
	atlas.add_to_atlas("player", "./res/mainChar/mage3.png", "./res/mainChar/mage3.json");
	atlas.add_to_atlas("enemy1", "./res/enemy1/enemy1.png", "./res/enemy1/enemy1.json");

	player.load(atlas);
	world.init("./res/world1.ldtk", game);
	world.load_assets(atlas);

	atlas.pack_atlas();
	gfx.upload_atlas(atlas);

	SDL_ShowWindow(game.window);
	int returnVal = main_loop();
	
	gfx.cleanup();
	atlas.destroy();
	SDL_DestroyWindow(game.window);
	GameContext::cleanup();
	mems::close();
	SDL_Quit();
}

int main_loop() {
	while (true) {
		// timer start - this is meant for framelimiting
		const Uint64 startFrame = SDL_GetTicksNS();

		// process events
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
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

		//
		// update
		//
		player.update(game);

		//
		// render
		//
		input.end_frame();
		gfx.begin_frame();

		world.render(gfx);
		player.render(gfx);
		gfx.queue_text(16, 16, "NES Game Jam!");

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