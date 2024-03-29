#include "SDL3/SDL.h"

#include "config.h"
#include "input.h"

struct Event {
	int sym;
	void (*fn)(bool);
};

inline static bool is_button_down(SDL_EventType event_type);

float mouse_x;
float mouse_y;

static int eventc;
static struct Event events[MAX_EVENT_CALLBACKS];

void input_poll(void)
{
	SDL_GetMouseState(&mouse_x, &mouse_y);
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_EVENT_QUIT:
			quit();
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
		case SDL_EVENT_MOUSE_WHEEL:
			for (int i = 0; i < eventc; i++)
				if (events[i].sym == event.key.keysym.sym || events[i].sym == event.button.button)
					events[i].fn(is_button_down(event.type));
			break;
		default:
			break;
		}
	}
}

void input_register(SDL_Keycode key, void (*fn)(bool))
{
	events[eventc++] = (struct Event){
		.sym  = key,
		.fn   = fn,
	};
	DEBUG(5, "[INPUT] Registered key %d", key);
}

void input_deregister(SDL_Keycode key, void (*fn)(bool))
{
	for (int i = 0; i < eventc; i++) {
		if (events[i].sym == key) {
			if (events[i].fn == fn) {
				events[i] = (struct Event){ 0 };
				return;
			} else {
				ERROR("[INPUT] Event for key %d does not match given function", key);
			}
		}
	}

	ERROR("[INPUT] Could not find event to deregister for key %d", key);
}

void input_reset()
{
	eventc = 0;
}

/* -------------------------------------------------------------------- */

inline static bool is_button_down(SDL_EventType event_type)
{
	return event_type == SDL_EVENT_KEY_DOWN || event_type == SDL_EVENT_MOUSE_BUTTON_DOWN;
}
