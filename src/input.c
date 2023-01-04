#include "SDL2/SDL.h"

#include "input.h"

static int keycbc;
static struct InputCallback keycbs[MAX_INPUT_CALLBACKS];
static void (*mousecbs[MOUSE_EVENT_COUNT])(int, bool, int, int);

bool input_check()
{
	void (*fn)(int, bool, int, int) = NULL;
	int btn = 0;
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				return true;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				for (int i = 0; i < keycbc; i++)
					if (keycbs[i].key == event.key.keysym.sym)
						if ((event.type == SDL_KEYDOWN && keycbs[i].onkeydown) ||
						    (event.type == SDL_KEYUP   && keycbs[i].onkeyup))
							keycbs[i].fn(event.type == SDL_KEYDOWN);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				switch (event.button.button) {
					case SDL_BUTTON_LEFT  : fn = mousecbs[MOUSE_LEFT  ]; btn = MOUSE_LEFT  ; break;
					case SDL_BUTTON_RIGHT : fn = mousecbs[MOUSE_RIGHT ]; btn = MOUSE_RIGHT ; break;
					case SDL_BUTTON_MIDDLE: fn = mousecbs[MOUSE_MIDDLE]; btn = MOUSE_MIDDLE; break;
				}
				if (fn)
					fn(btn, event.button.state == SDL_PRESSED, event.button.x, event.button.y);
				break;
			case SDL_MOUSEMOTION:
				fn = mousecbs[MOUSE_DRAG];
				if (fn)
					fn(MOUSE_DRAG, false, event.button.x, event.button.y);
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

void input_register_mouse(enum MouseInput btn, void (*fn)(int, bool, int, int))
{
	if (btn == MOUSE_NONE || btn >= MOUSE_INVALID)
		ERROR("[INPUT] %d is not a valid mouse button", btn);
	else
		mousecbs[btn] = fn;
}
