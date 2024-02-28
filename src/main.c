#include <vulkan/vulkan.h>
#include "SDL3/SDL.h"

#include "SDL3/SDL_init.h"
#include "config.h"
#include "taskmgr.h"
#include "gfx/vulkan.h"
#include "gfx/renderer.h"
#include "gfx/ui/ui.h"
#include "camera.h"
#include "input.h"
#include "entities/entity.h"
#include "scenemgr.h"

#include "gfx/particles.h"
#include "test.h"

void init_sdl(void);

struct GlobalConfig global_config;
SDL_Renderer* renderer;
SDL_Window*   window;
int vk_err;

#ifndef TESTING
int main(int argc, char** argv)
{
	for (int i = 0; i < argc; i++)
		printf("%s\n", argv[i]);

	// TODO
	global_config.winw = 1280;
	global_config.winh = 720;

	init_sdl();

	DEBUG(1, "[INFO] Platform: %s", SDL_GetPlatform());
	DEBUG(1, "[INFO] CPU info:");
	DEBUG(1, "\tLogical cores: %d", SDL_GetCPUCount());
	DEBUG(1, "\tL1 Cache Line: %dB", SDL_GetCPUCacheLineSize());
	DEBUG(1, "\tRAM          : %.1fGB", SDL_GetSystemRAM()/1024.0);

	uint32 vkversion;
	vkEnumerateInstanceVersion(&vkversion);
	DEBUG(1, "[INFO] Vulkan version: %u.%u.%u", VK_API_VERSION_MAJOR(vkversion),
	      VK_API_VERSION_MINOR(vkversion), VK_API_VERSION_PATCH(vkversion));

	srand(SDL_GetTicks());

	init_vulkan(window);
	scenemgr_init();

	scenemgr_loop();

	return 0;
}
#endif /* TESTING */

void init_sdl()
{
	SDL_Version sdlversion;
	SDL_GetVersion(&sdlversion);
	DEBUG(1, "[INFO] SDL version: %u.%u.%u", sdlversion.major, sdlversion.minor, sdlversion.patch);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
		ERROR("[INIT] Failed to initialize SDL (%s)", SDL_GetError());
	else
		DEBUG(1, "[INIT] Initialized SDL");

	window = SDL_CreateWindow("CSage", global_config.winw, global_config.winh, SDL_WINDOW_VULKAN);
	if (!window)
		ERROR("[INIT] Failed to create window (%s)", SDL_GetError());
	else
		DEBUG(1, "[INIT] Created window");
}

noreturn void quit()
{
	DEBUG(1, "/------------------------------\\");
	DEBUG(1, "|        Cleaning up...        |");
	DEBUG(1, "\\------------------------------/");
	renderer_free();
	entities_free();
	vulkan_free();
	SDL_Quit();
	exit(0);
}

