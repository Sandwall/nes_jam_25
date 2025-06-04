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

//
void Input::handle_events(const SDL_Event& event) {
	switch (event.type) {
		case SDL_EVENT_KEY_DOWN:

			break;
		case SDL_EVENT_KEY_UP:
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