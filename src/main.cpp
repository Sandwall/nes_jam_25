#include <SDL3/SDL.h>

#include "input.h"
#include "gfx.h"

#include <stdio.h>
#include <math.h>
#include <tinydef.hpp>

constexpr int defWidth = Gfx::nesWidth * 4, defHeight = Gfx::nesHeight * 4;

Uint64 targetFps = 60;

Uint64 target_ns() {
	if (targetFps == 0) return 0;
	return SDL_NS_PER_SECOND / targetFps;
}

int main(int argc, char** argv) {
	if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD))
		return -1;

	SDL_Window* window = SDL_CreateWindow("NES Game!", defWidth, defHeight, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
	SDL_SetWindowMinimumSize(window, Gfx::nesWidth * 2, Gfx::nesHeight * 2);
	Gfx gfx(window);
	gfx.init();
	SDL_ShowWindow(window);

	Input input;
	bool windowStaysAlive = true;
	float deltaTime = 1.0f / static_cast<float>(targetFps);

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

		//
		// update
		//

		// these test variables are defined as static to keep them around the logic that changes them
		// we would not normally do this for the actual gameobjects, but we're keeping it this way
		// just because this is a demonstration/test
		static float hsvTimer = 0.0f;
		static SDL_FRect rectPos = { 128.0f, 128.0f, 16.0f, 16.0f };
		static float rectVelocityX = 60.0f, rectVelocityY = 30.0f;

		// modulate clear color according to hsv
		hsvTimer = fmodf(hsvTimer + (deltaTime * 60.0f), 360.0f);
		gfx.clearColor = Gfx::hsv_to_col(hsvTimer, 0.2f, 1.0f, 1.0f);

		// bounding the rectangle around like the DVD logo
		rectPos.x += rectVelocityX * deltaTime;
		rectPos.y += rectVelocityY * deltaTime;
		if (rectPos.x + rectPos.w > static_cast<float>(Gfx::nesWidth)) {
			rectPos.x = static_cast<float>(Gfx::nesWidth) - rectPos.w;
			rectVelocityX = -rectVelocityX;
		} else if (rectPos.x < 0.0f) {
			rectPos.x = 0.0f;
			rectVelocityX = -rectVelocityX;
		}
		
		if (rectPos.y + rectPos.h > static_cast<float>(Gfx::nesHeight)) {
			rectPos.y = static_cast<float>(Gfx::nesHeight) - rectPos.h;
			rectVelocityY = -rectVelocityY;
		} else if (rectPos.y < 0.0f) {
			rectPos.y = 0.0f;
			rectVelocityY = -rectVelocityY;
		}

		//
		// render
		//
		input.end_frame();
		gfx.begin_frame();
		gfx.queue_rect(rectPos, SDL_FColor{1.0f, 0.0f, 1.0f, 1.0});

		gfx.finish_frame();
		
		// end frame - framelimiting logic
		Uint64 elapsed = SDL_GetTicksNS() - startFrame;
		if (target_ns() > elapsed) {
			// keep in mind these values are unsigned, we do not want to have an underflow to UINT64_MAX
			SDL_DelayNS(target_ns() - elapsed);
		}

		// calculate delta time
		elapsed = SDL_GetTicksNS() - startFrame;
		deltaTime = SDL_NS_TO_SECONDS(static_cast<float>(elapsed));
	}
	
	gfx.cleanup();
	SDL_DestroyWindow(window);
	SDL_Quit();
}