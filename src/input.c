#include "SDL2/SDL.h"

#include "input.h"
#include <SDL2/SDL_events.h>

static uint keycbc;
static struct InputCallback keycbs[MAX_INPUT_CALLBACKS];
static void (*mousecbs[MOUSE_BUTTON_COUNT])(bool);

bool input_check()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				return true;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				for (uint i = 0; i < keycbc; i++)
					if (keycbs[i].key == event.key.keysym.sym)
						if ((event.type == SDL_KEYDOWN && keycbs[i].onkeydown) ||
						    (event.type == SDL_KEYUP   && keycbs[i].onkeyup))
							keycbs[i].fn(event.type == SDL_KEYDOWN);
				break;
			case SDL_MOUSEBUTTONDOWN:
				void (*fn)(bool);
				switch (event.button.button) {
					case SDL_BUTTON_LEFT  : fn = mousecbs[MOUSE_BUTTON_LEFT  ]; break;
					case SDL_BUTTON_RIGHT : fn = mousecbs[MOUSE_BUTTON_RIGHT ]; break;
					case SDL_BUTTON_MIDDLE: fn = mousecbs[MOUSE_BUTTON_MIDDLE]; break;
				}
				if (fn)
					fn(event.button.state == SDL_PRESSED);
		}
	}
	return false;
}

void input_register_key(struct InputCallback cb)
{
	keycbs[keycbc++] = cb;
	DEBUG(3, "[INPUT] Key %d registered for keydown:%s, keyup:%s", cb.key,
	      STRING_TF(cb.onkeydown), STRING_TF(cb.onkeyup));
}

void input_register_mouse(enum MouseButton btn, void (*fn)(bool))
{
	if (btn == MOUSE_BUTTON_NONE || btn >= MOUSE_BUTTON_INVALID)
		ERROR("[INPUT] %d is not a valid mouse button", btn);
	else
		mousecbs[btn] = fn;
}
