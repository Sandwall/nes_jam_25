#pragma once

#include <SDL3/SDL_events.h>

struct InputAction {
	enum Type {
		UP = 0,
		DOWN,
		LEFT,
		RIGHT,
		A,
		B,
		START,
		SELECT,
		MAX
	};

	// This gets set by some input handler
	bool down :  1 = false;

	// This gets set at the end of each frame
	bool prevDown : 1 = false;

	operator bool() const;

	inline void end_frame();
	bool clicked() const;
	bool released() const;
};

struct Input {
	Input();

	union {
		struct {
			InputAction up;
			InputAction left;
			InputAction down;
			InputAction right;
			InputAction a;
			InputAction b;
			InputAction select;
			InputAction start;
		};

		InputAction actions[8];
	};

	SDL_Scancode keyBindings[InputAction::MAX] = {
		SDL_SCANCODE_UP,		// up
		SDL_SCANCODE_LEFT,		// left
		SDL_SCANCODE_DOWN,		// down
		SDL_SCANCODE_RIGHT,		// right
		SDL_SCANCODE_Z,			// a
		SDL_SCANCODE_X,			// b
		SDL_SCANCODE_RSHIFT,	// select
		SDL_SCANCODE_RETURN,	// start
	};

	void end_frame();
	void handle_event(const SDL_Event& event);
};
