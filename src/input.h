#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>

#define MAX_INPUT_CALLBACKS 16
#define MOUSE_BUTTON_COUNT  3

enum MouseButton {
	MOUSE_BUTTON_NONE,
	MOUSE_BUTTON_LEFT,
	MOUSE_BUTTON_RIGHT,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_INVALID,
};

struct InputCallback {
	void (*fn)(bool);
	int  key;
	bool onkeydown;
	bool onkeyup;
}; static_assert(sizeof(struct InputCallback) == 16, "struct InputCallback");

bool input_check();
void input_register_key(struct InputCallback cb);
void input_register_mouse(enum MouseButton btn, void (*fn)(bool, int, int));

#endif
