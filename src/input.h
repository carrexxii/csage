#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>

#define MAX_INPUT_CALLBACKS 16
#define MOUSE_EVENT_COUNT   MOUSE_INVALID
#define MAX_CALLBACKS_PER_MOUSE_BUTTON 5

enum MouseInput {
	MOUSE_LEFT,
	MOUSE_RIGHT,
	MOUSE_MIDDLE,
	MOUSE_DRAG,
	MOUSE_INVALID,
	MOUSE_NONE,
};

struct KeyboardCallback {
	void (*fn)(bool);
	int  key;
	bool onkeydown;
	bool onkeyup;
};

bool input_check();
void input_register_key(struct KeyboardCallback cb);
/* Mouse inputs will keep a list of functions registered for each key.
 * The functions will be called in order - stopping if one of them returns `true`.
 * ie - a function returning `false` means that we proceed to the next callback.
 */
void input_register_mouse(enum MouseInput btn, bool (*fn)(int, bool, int, int));

#endif

