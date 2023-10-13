#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "config.h"
#include "taskmgr.h"
#include "gfx/vulkan.h"
#include "gfx/renderer.h"
#include "camera.h"
#include "input.h"
#include "entities/player.h"

#include "gfx/particles.h"
#include "test.h"

void init_sdl();
void init_input();
noreturn void quit_cb(bool kdown);

SDL_Renderer* renderer;
SDL_Window*   window;

uint config_window_width  = 1280;
uint config_window_height = 720;

int main(int argc, char** argv)
{
	for (int i = 0; i < argc; i++)
		printf("%s\n", argv[i]);

	init_sdl();

	uint32 vkversion;
	vkEnumerateInstanceVersion(&vkversion);
	DEBUG(1, "[INFO] Vulkan version: %u.%u.%u", VK_API_VERSION_MAJOR(vkversion),
	      VK_API_VERSION_MINOR(vkversion), VK_API_VERSION_PATCH(vkversion));

	init_input();
	init_vulkan(window);
	renderer_init();
	camera_init();
	player_init();

	srand(SDL_GetTicks64());

	taskmgr_init();
	taskmgr_add_task(camera_update);
	taskmgr_add_task(particles_update);
	taskmgr_add_task(player_update);

	test_init();

	DEBUG(1, "\nBeginning main loop (load time: %lums)\n"
	           "--------------------------------------", SDL_GetTicks64());
	uint64 delta, newtime, oldtime = 0.0, accum = 0.0;
	while (!input_check()) {
		newtime = SDL_GetTicks64();
		delta   = newtime - oldtime;
		oldtime = newtime;
		accum  += delta;
		while (accum >= dt_ms) {
			while (!taskmgr_reset());
			accum -= dt_ms;
		}
		renderer_draw();
	}

	return 0;
}

void init_sdl()
{
	SDL_version sdlversion;
	SDL_GetVersion(&sdlversion);
	DEBUG(1, "[INFO] SDL version: %u.%u.%u", sdlversion.major, sdlversion.minor, sdlversion.patch);

	if (SDL_Init(SDL_INIT_EVERYTHING))
		ERROR("[INIT] Failed to initialize SDL (%s)", SDL_GetError());
	else
		DEBUG(1, "[INIT] Initialized SDL");

	window = SDL_CreateWindow("CSage", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                          config_window_width, config_window_height, SDL_WINDOW_VULKAN);
	if (!window)
		ERROR("[INIT] Failed to create window (%s)", SDL_GetError());
	else
		DEBUG(1, "[INIT] Created window");
}

void init_input()
{
	input_register_key((struct KeyboardCallback){ .key = SDLK_ESCAPE  , .fn = quit_cb, .onkeydown = true });
	input_register_key((struct KeyboardCallback){ .key = SDLK_RIGHT   , .fn = camera_move_right_cb, .onkeydown = true, .onkeyup = true, });
	input_register_key((struct KeyboardCallback){ .key = SDLK_LEFT    , .fn = camera_move_left_cb , .onkeydown = true, .onkeyup = true, });
	input_register_key((struct KeyboardCallback){ .key = SDLK_UP      , .fn = camera_move_up_cb   , .onkeydown = true, .onkeyup = true, });
	input_register_key((struct KeyboardCallback){ .key = SDLK_DOWN    , .fn = camera_move_down_cb , .onkeydown = true, .onkeyup = true, });
	input_register_key((struct KeyboardCallback){ .key = SDLK_PAGEUP  , .fn = camera_zoom_in_cb   , .onkeydown = true, .onkeyup = true, });
	input_register_key((struct KeyboardCallback){ .key = SDLK_PAGEDOWN, .fn = camera_zoom_out_cb  , .onkeydown = true, .onkeyup = true, });

	input_register_key((struct KeyboardCallback){ .key = SDLK_w, .fn = player_move_up_cb   , .onkeydown = true, .onkeyup = true });
	input_register_key((struct KeyboardCallback){ .key = SDLK_a, .fn = player_move_left_cb , .onkeydown = true, .onkeyup = true });
	input_register_key((struct KeyboardCallback){ .key = SDLK_s, .fn = player_move_down_cb , .onkeydown = true, .onkeyup = true });
	input_register_key((struct KeyboardCallback){ .key = SDLK_d, .fn = player_move_right_cb, .onkeydown = true, .onkeyup = true });
}

noreturn void quit_cb(bool kdown)
{
	DEBUG(1, "/------------------------------\\");
	DEBUG(1, "|        Cleaning up...        |");
	DEBUG(1, "\\------------------------------/");
	renderer_free();
	map_free();
	player_free();
	free_vulkan();
	SDL_Quit();
	exit(0);
}

