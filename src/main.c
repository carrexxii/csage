#include <vulkan/vulkan.h>
#include "SDL3/SDL.h"

#define CLIB_IMPLEMENTATION
#include "clib/clib.h"
#include "common.h"

#include "resmgr.h"
#include "lua.h"
#include "taskmgr.h"
#include "gfx/vulkan.h"
#include "gfx/renderer.h"
#include "gfx/ui/ui.h"
#include "camera.h"
#include "input.h"
#include "entities/entity.h"
#include "scenemgr.h"

#include "gfx/particles.h"

void init_sdl(void);

SDL_Window* window;
int vk_err;

#ifndef TESTING
int main(int argc, char** argv)
{
	for (int i = 0; i < argc; i++)
		printf("%s\n", argv[i]);

	resmgr_init();
	lua_init();
	init_sdl();

	INFO(TERM_DARK_YELLOW "[INFO] Platform: %s", SDL_GetPlatform());
	INFO(TERM_DARK_YELLOW "[INFO] CPU info:");
	INFO("\tLogical cores -> %d", SDL_GetCPUCount());
	INFO("\tL1 Cache Line -> %dB", SDL_GetCPUCacheLineSize());
	INFO("\tRAM           -> %.1fGB", SDL_GetSystemRAM() / 1024.0f);

	uint32 vkversion;
	vkEnumerateInstanceVersion(&vkversion);
	INFO(TERM_DARK_YELLOW "[INFO] Vulkan version: %u.%u.%u", VK_API_VERSION_MAJOR(vkversion),
	      VK_API_VERSION_MINOR(vkversion), VK_API_VERSION_PATCH(vkversion));

	srand(SDL_GetTicks());

	init_vulkan(window);
	scenemgr_init(window);

	scenemgr_loop();

	return 0;
}
#endif /* TESTING */

void init_sdl()
{
	SDL_Version sdlversion;
	SDL_GetVersion(&sdlversion);
	INFO(TERM_DARK_YELLOW "[INFO] SDL version: %u.%u.%u", sdlversion.major, sdlversion.minor, sdlversion.patch);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
		ERROR("[INIT] Failed to initialize SDL (%s)", SDL_GetError());
	else
		INFO(TERM_DARK_YELLOW "[INIT] Initialized SDL");

	window = SDL_CreateWindow("CSage", config.winw, config.winh, SDL_WINDOW_VULKAN);
	if (!window)
		ERROR("[INIT] Failed to create window (%s)", SDL_GetError());
	else
		INFO(TERM_DARK_YELLOW "[INIT] Created window");
}

[[noreturn]]
void quit()
{
	INFO(TERM_DARK_YELLOW "/------------------------------\\");
	INFO(TERM_DARK_YELLOW "|        Cleaning up...        |");
	INFO(TERM_DARK_YELLOW "\\------------------------------/");
	vkDeviceWaitIdle(logical_gpu);

	entities_free();
	scenemgr_free();
	lua_free();
	resmgr_free();
	renderer_free();
	vulkan_free();
	SDL_Quit();
	exit(0);
}

