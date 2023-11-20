#include "SDL2/SDL.h"

#include "config.h"
#include "input.h"

struct Event {
	SDL_EventType type;
	int sym;
	void (*fn)(void);
};

int mouse_x;
int mouse_y;

static int eventc;
static struct Event events[MAX_EVENT_CALLBACKS];

void input_poll(void)
{
	SDL_GetMouseState(&mouse_x, &mouse_y);
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			quit();
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEMOTION:
		case SDL_MOUSEWHEEL:
			for (int i = 0; i < eventc; i++)
				if (events[i].type == event.type) {
					if (events[i].sym == event.key.keysym.sym &&
					    (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP))
						events[i].fn();
					else if (events[i].sym == event.button.button)
						events[i].fn();
				}
			break;
		default:
			break;
		}
	}
}

void input_register(SDL_EventType type, SDL_Keycode key, void (*fn)(void)) {
	events[eventc++] = (struct Event){
		.type = type,
		.sym  = key,
		.fn   = fn,
	};
	DEBUG(5, "[INPUT] Register key %d with event type %d", key, type);
}

void input_deregister(SDL_EventType type, SDL_Keycode key, void (*fn)(void))
{
	for (int i = 0; i < eventc; i++)
		if (events[i].type == type && events[i].sym == key) {
			if (events[i].fn == fn)
				return events[i] = (struct Event){ 0 }, (void)0;
			else
				ERROR("[INPUT] Event for key %d does not match given function", key);
		}

	ERROR("[INPUT] Could not find event to deregister for event %d, key %d", type, key);
}
