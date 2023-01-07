#include "SDL2/SDL.h"

#include "input.h"

static int keycbc;
static struct KeyboardCallback keycbs[MAX_INPUT_CALLBACKS];
static bool (*mousecbs[MOUSE_EVENT_COUNT][MAX_CALLBACKS_PER_MOUSE_BUTTON])(int, bool, int, int);

bool input_check()
{
	bool (**fns)(int, bool, int, int) = NULL;
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
					case SDL_BUTTON_LEFT  : fns = mousecbs[MOUSE_LEFT  ]; btn = MOUSE_LEFT  ; break;
					case SDL_BUTTON_RIGHT : fns = mousecbs[MOUSE_RIGHT ]; btn = MOUSE_RIGHT ; break;
					case SDL_BUTTON_MIDDLE: fns = mousecbs[MOUSE_MIDDLE]; btn = MOUSE_MIDDLE; break;
				}
				for (int i = 0; i < MAX_CALLBACKS_PER_MOUSE_BUTTON; i++) {
					if (fns[i])
						if (fns[i](btn, event.button.state == SDL_PRESSED, event.button.x, event.button.y))
							break;
				}
				break;
			case SDL_MOUSEMOTION:
				fns = mousecbs[MOUSE_DRAG];
				for (int i = 0; i < MAX_CALLBACKS_PER_MOUSE_BUTTON; i++) {
					if (fns[i])
						if (fns[i](MOUSE_DRAG, false, event.button.x, event.button.y))
							break;
				}
		}
	}
	return false;
}

void input_register_key(struct KeyboardCallback cb)
{
	keycbs[keycbc++] = cb;
	DEBUG(3, "[INPUT] Key %d registered for keydown:%s, keyup:%s", cb.key,
	      STRING_TF(cb.onkeydown), STRING_TF(cb.onkeyup));
}

void input_register_mouse(enum MouseInput btn, bool (*fn)(int, bool, int, int))
{
	if (btn == MOUSE_NONE || btn >= MOUSE_INVALID) {
		ERROR("[INPUT] %d is not a valid mouse button", btn);
	} else {
		for (int i = 0; i < MAX_CALLBACKS_PER_MOUSE_BUTTON; i++) {
			if (!mousecbs[btn][i]) {
				mousecbs[btn][i] = fn;
				
				DEBUG(3, "[INPUT] Registered mouse button %d (%d total for this button)", btn, i + 1);
				return;
			}
		}
		ERROR("[INPUT] Exceeded the max number of functions bounded to mouse button %d", btn);
	}
}

