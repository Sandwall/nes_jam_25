#include <SDL3/SDL.h>

#include "engine/input.h"
#include "engine/gfx.h"
#include "engine/game_context.h"
#include "engine/image_asset.h"
#include "engine/audio.h"

#include "game/player.h"
#include "game/enemy.h"
#include "game/world.h"
#include "game.h"

#include <stdio.h>

#include <tinydef.hpp>
#include <mems.hpp>
#include <vector>

constexpr int defWidth = Gfx::nesWidth * 4, defHeight = Gfx::nesHeight * 4;
int windowWidth = defWidth, windowHeight = defHeight;    // these are here if we ever want it for some reason

Input input;
Gfx gfx;
TextureAtlas atlas;
GameWorld world;

Player player;
std::vector<Enemy> enemies;

bool paused = false;
bool inMainMenu = true;

GameContext game = {
	.delta = 1.0f / 60.0f,
	.points = 0,
	.targetFps = 60,
	.gfx = &gfx,
	.input = &input,
	.atlas = &atlas,
	.world = &world,
	.player = &player,
};

int main(int argc, char** argv) {
	//
	// INIT
	//
	if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD))
		return -1;

	mems::init();
	GameContext::init();

	// create window and init graphics
	game.window = SDL_CreateWindow("NES Game!", windowWidth, windowHeight, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
	SDL_SetWindowMinimumSize(game.window, Gfx::nesWidth, Gfx::nesHeight);
	if (!gfx.init(game.window))
		return -1;

	game_init();
	audio_init();

	SDL_ShowWindow(game.window);

	//
	// MAIN LOOP
	//
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
				[[fallthrough]];
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

		// Check if the player is in main menu
		if (inMainMenu)
		{
			if (input.start.clicked())
			{
				input.start.down = false; // Prevents pausing the game immediately
				input.start.prevDown = true; // Set start to its previous key/button press

				inMainMenu = false; // Switch to game state and out of main menu
			}

			gfx.begin_frame();

			// Render game title text
			gfx.queue_text(Gfx::nesWidth / 2.8, Gfx::nesHeight / 8, "NES Game!");

			// Render press start text
			gfx.queue_text(Gfx::nesWidth / 5, Gfx::nesHeight / 2, "Press Start to play!");

			gfx.finish_frame();
		}

		// Otherwise, check if the player is playing the game
		else
		{
			if (input.start.clicked())
				paused = !paused;

			//
			// update
			//
			if (!paused) game_update();

			input.end_frame();
			audio_tick();

			//
			// render
			//
			gfx.begin_frame();

			game_render();

			static char pointStr[32];
			snprintf(pointStr, 32, "PTS %d", game.points);
			gfx.queue_text(5, 5, pointStr);

			if (paused) {
				constexpr int PAUSED_TEXT_X = (Gfx::nesWidth / 2) - (static_cast<int>(std::char_traits<char>::length("PAUSED") * 8) / 2);
				constexpr int PAUSED_TEXT_Y = (Gfx::nesHeight / 2) - 16;
				gfx.queue_text(PAUSED_TEXT_X, PAUSED_TEXT_Y, "PAUSED");
			}

			gfx.finish_frame();
		}

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

	//
	// CLEANUP
	//
	audio_close();
	gfx.cleanup();
	atlas.destroy();
	SDL_DestroyWindow(game.window);
	GameContext::cleanup();
	mems::close();
	SDL_Quit();
}
