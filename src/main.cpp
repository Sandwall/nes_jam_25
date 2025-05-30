#include <SDL3/SDL.h>

#include "input.h"
#include "gfx.h"

constexpr int defWidth = nesWidth * 4, defHeight = nesHeight * 4;

Uint64 targetFps = 60;

Uint64 target_ns() {
	if (targetFps == 0) return 0;
	return SDL_NS_PER_SECOND / static_cast<double>(targetFps);
}

int main(int argc, char** argv) {
	if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD))
		return -1;

	SDL_Window* window = SDL_CreateWindow("NES Game!", defWidth, defHeight, SDL_WINDOW_HIDDEN);
	SDL_SetWindowMinimumSize(window, nesWidth * 2, nesHeight * 2);
	
	Gfx gfx(window);
	gfx.init();
	
	SDL_ShowWindow(window);

	Input input;
	bool windowStaysAlive = true;
	while (windowStaysAlive) {
		// timer start - this is meant for framelimiting
		const Uint64 startFrame = SDL_GetTicksNS();

		// process events
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_EVENT_QUIT:
				windowStaysAlive = false;
				break;
			case SDL_EVENT_KEY_DOWN:
			case SDL_EVENT_KEY_UP:
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
			case SDL_EVENT_MOUSE_MOTION:
			case SDL_EVENT_GAMEPAD_AXIS_MOTION:
			case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			case SDL_EVENT_GAMEPAD_BUTTON_UP:
				input.handle_events(event);
				break;
			}
		}

		// skip frame if we get the signal to quit the window
		if (!windowStaysAlive) break;


		// update

		// render


		input.end_frame();
		
		// end frame - framelimiting logic
		Uint64 elapsed = static_cast<double>(SDL_GetTicksNS() - startFrame);
		if (target_ns() > elapsed) {
			// keep in mind these values are unsigned, we do not want to have an underflow to UINT64_MAX
			SDL_DelayNS(target_ns() - elapsed);
		}
	}
	
	gfx.cleanup();
	SDL_DestroyWindow(window);
	SDL_Quit();
}