#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include "cglm/cglm.h"

#include "common.h"
#include "config.h"
#include "gfx/vulkan.h"
#include "gfx/renderer.h"
#include "camera.h"
#include "input.h"

void init_sdl();
void init_input();
noreturn void quit_cb(bool kdown);

SDL_Renderer* renderer;
SDL_Window*   window;

int main(int argc, char** argv)
{
	for (int i = 0; i < argc; i++)
		printf("%s\n", argv[i]);

	uint32 vkversion;
	vkEnumerateInstanceVersion(&vkversion);
	DEBUG(1, "[INFO] Vulkan version: %u.%u.%u", VK_API_VERSION_MAJOR(vkversion),
	      VK_API_VERSION_MINOR(vkversion), VK_API_VERSION_PATCH(vkversion));

	init_sdl();
	init_input();
	renderer_init(window);
	
	while (1) {
		if (check_input())
			break;
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
	register_key((struct InputCallback){ .key = SDLK_ESCAPE, .fn = quit_cb, .onkeydown = true });
	register_key((struct InputCallback){ .key = SDLK_d, .fn = move_camera_right_cb, .onkeydown = true });
	register_key((struct InputCallback){ .key = SDLK_a, .fn = move_camera_left_cb , .onkeydown = true });
	register_key((struct InputCallback){ .key = SDLK_w, .fn = move_camera_up_cb   , .onkeydown = true });
	register_key((struct InputCallback){ .key = SDLK_s, .fn = move_camera_down_cb , .onkeydown = true });
	register_key((struct InputCallback){ .key = SDLK_q, .fn = zoom_camera_in_cb   , .onkeydown = true });
	register_key((struct InputCallback){ .key = SDLK_e, .fn = zoom_camera_out_cb  , .onkeydown = true });
}

noreturn void quit_cb(bool kdown)
{
	DEBUG(1, TERM_GREEN "|------------------------------|");
	DEBUG(1, TERM_GREEN "|\tCleaning up...         |");
	DEBUG(1, TERM_GREEN "|------------------------------|");
	renderer_free();
	SDL_Quit();
	exit(0);
}
