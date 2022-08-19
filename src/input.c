#include "SDL2/SDL.h"

#include "input.h"

static uint cbc;
static struct InputCallback cbs[MAX_INPUT_CALLBACKS];

bool check_input()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				return true;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				for (uint i = 0; i < cbc; i++)
					if (cbs[i].key == event.key.keysym.sym)
						if ((event.type == SDL_KEYDOWN && cbs[i].onkeydown) ||
						    (event.type == SDL_KEYUP   && cbs[i].onkeyup))
							cbs[i].fn(event.type == SDL_KEYDOWN);
				break;
		}
	}
	return false;
}

void register_key(struct InputCallback cb)
{
	cbs[cbc++] = cb;
	DEBUG(3, "[INPUT] Key %d registered for keydown:%s, keyup:%s", cb.key,
	      STRING_TF(cb.onkeydown), STRING_TF(cb.onkeyup));
}
