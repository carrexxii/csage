#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "config.h"
#include "taskmgr.h"
#include "gfx/vulkan.h"
#include "gfx/renderer.h"
#include "camera.h"
#include "input.h"

#include "gfx/polygon.h"
#include "ships/ship.h"
#include "test.h"

void init_sdl();
void init_input();
noreturn void quit_cb(bool kdown);

SDL_Renderer* renderer;
SDL_Window*   window;

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
	ships_init();

	srand(SDL_GetTicks64());

	taskmgr_init();
	taskmgr_add_task(camera_update);
	taskmgr_add_task(ships_update);

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
	                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_VULKAN);
	if (!window)
		ERROR("[INIT] Failed to create window (%s)", SDL_GetError());
	else
		DEBUG(1, "[INIT] Created window");
}

void init_input()
{
	input_register_key((struct KeyboardCallback){ .key = SDLK_ESCAPE, .fn = quit_cb, .onkeydown = true });
	input_register_key((struct KeyboardCallback){ .key = SDLK_d, .fn = camera_move_right_cb, .onkeydown = true, .onkeyup = true, });
	input_register_key((struct KeyboardCallback){ .key = SDLK_a, .fn = camera_move_left_cb , .onkeydown = true, .onkeyup = true, });
	input_register_key((struct KeyboardCallback){ .key = SDLK_w, .fn = camera_move_up_cb   , .onkeydown = true, .onkeyup = true, });
	input_register_key((struct KeyboardCallback){ .key = SDLK_s, .fn = camera_move_down_cb , .onkeydown = true, .onkeyup = true, });
	input_register_key((struct KeyboardCallback){ .key = SDLK_q, .fn = camera_zoom_in_cb   , .onkeydown = true, .onkeyup = true, });
	input_register_key((struct KeyboardCallback){ .key = SDLK_e, .fn = camera_zoom_out_cb  , .onkeydown = true, .onkeyup = true, });

	input_register_key((struct KeyboardCallback){ .key = SDLK_o, .fn = test_o_cb, .onkeydown = true, });
	input_register_key((struct KeyboardCallback){ .key = SDLK_p, .fn = test_p_cb, .onkeydown = true, });
	input_register_key((struct KeyboardCallback){ .key = SDLK_k, .fn = test_k_cb, .onkeydown = true, });
	input_register_key((struct KeyboardCallback){ .key = SDLK_l, .fn = test_l_cb, .onkeydown = true, });
}

noreturn void quit_cb(bool kdown)
{
	DEBUG(1, "/------------------------------\\");
	DEBUG(1, "|        Cleaning up...        |");
	DEBUG(1, "\\------------------------------/");
	renderer_free();
	ships_free();
	free_vulkan();
	SDL_Quit();
	exit(0);
}

