#pragma once

#include <SDL3/SDL_events.h>

struct InputAction {
	enum Type {
		NONE = 0,
		UP,
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
	InputAction up, left, down, right;
	InputAction a, b;
	InputAction select, start;

	SDL_Scancode bindings[InputAction::MAX] = {
		SDL_SCANCODE_UP,
		SDL_SCANCODE_DOWN,
		SDL_SCANCODE_LEFT,
		SDL_SCANCODE_RIGHT,
		SDL_SCANCODE_Z,
		SDL_SCANCODE_X,
		SDL_SCANCODE_RETURN,
		SDL_SCANCODE_RSHIFT
	};

	void end_frame();
	void handle_events(const SDL_Event& event);
};
