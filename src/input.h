#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>

#define MAX_INPUT_CALLBACKS 16

struct InputCallback {
	void (*fn)(bool);
	int key;
	bool onkeydown;
	bool onkeyup;
}; static_assert(sizeof(struct InputCallback) == 16, "struct InputCallback");

bool input_check();
void input_register_key(struct InputCallback cb);

#endif
