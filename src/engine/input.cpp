#include "input.h"

//
// INPUT ACTION
//

InputAction::operator bool() const {
	return down;
}

void InputAction::end_frame() {
	prevDown = down;
}

bool InputAction::clicked() const {
	return !prevDown && down;
}

bool InputAction::released() const {
	return prevDown && !down;
}

//
// INPUT
//

// NOTE(sand): for some reason msvc generates a deleted constructor when we don't define a constructor manually 
Input::Input() {}

void Input::end_frame() {
	up.end_frame();
	down.end_frame();
	left.end_frame();
	right.end_frame();
	a.end_frame();
	b.end_frame();
	select.end_frame();
	start.end_frame();
}

// TODO(sand): instead of iterating 8 times (InputAction::MAX) for each event we get, we could maintain a buffer of
// at most 16 pairs of input actions and bools, queue "simpler" events to this buffer, and iterate over this buffer
void Input::handle_event(const SDL_Event& event) {
	switch (event.type) {
		case SDL_EVENT_KEY_DOWN:
			for (int i = 0; i < InputAction::MAX; i++) {
				if (event.key.scancode == keyBindings[i]) {
					actions[i].down = true;
					break;
				}
			}

			break;
		case SDL_EVENT_KEY_UP:
			for (int i = 0; i < InputAction::MAX; i++) {
				if (event.key.scancode == keyBindings[i]) {
					actions[i].down = false;
					break;
				}
			}
			break;

// not handling mouse/gamepad for now...
/*
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
		case SDL_EVENT_MOUSE_MOTION:
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
			break;
*/
	}
}