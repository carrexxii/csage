#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>

#define MAX_INPUT_CALLBACKS 16
#define MOUSE_EVENT_COUNT   MOUSE_INVALID

enum MouseInput {
	MOUSE_LEFT,
	MOUSE_RIGHT,
	MOUSE_MIDDLE,
	MOUSE_DRAG,
	MOUSE_INVALID,
	MOUSE_NONE,
};

struct InputCallback {
	void (*fn)(bool);
	int  key;
	bool onkeydown;
	bool onkeyup;
}; static_assert(sizeof(struct InputCallback) == 16, "struct InputCallback");

bool input_check();
void input_register_key(struct InputCallback cb);
void input_register_mouse(enum MouseInput btn, void (*fn)(int, bool, int, int));

#endif
