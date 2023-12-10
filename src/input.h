#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>

#define MAX_EVENT_CALLBACKS 64

/* !! Does not match with `SDL_BUTTON_*` values */
enum MouseMask {
	MOUSE_MASK_NONE       = 0x00,
	MOUSE_MASK_LEFT       = 0x01,
	MOUSE_MASK_RIGHT      = 0x02,
	MOUSE_MASK_MIDDLE     = 0x04,
	MOUSE_MASK_MOUSE_DRAG = 0x08,
};

void input_poll(void);
void input_register(SDL_Keycode key, void (*fn)(bool));
void input_deregister(SDL_Keycode key, void (*fn)(bool));
void input_reset(void);

extern int mouse_x;
extern int mouse_y;

#endif

