#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include "cglm/cglm.h"

#include "common.h"
#include "config.h"
#include "gfx/vulkan.h"
#include "gfx/renderer.h"

SDL_Renderer* renderer;
SDL_Window* window;

int main(int argc, char** argv)
{
	for (int i = 0; i < argc; i++)
		printf("%s\n", argv[i]);

	uint32 vkversion;
	vkEnumerateInstanceVersion(&vkversion);
	DEBUG(1, "[INFO] Vulkan version: %u.%u.%u", VK_API_VERSION_MAJOR(vkversion),
	      VK_API_VERSION_MINOR(vkversion), VK_API_VERSION_PATCH(vkversion));

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
	renderer_init(window);
	
	SDL_Event event;
	while (1) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					goto quit;
			}
		}
		renderer_draw();
	}

quit:
	DEBUG(1, TERM_GREEN "|------------------------------|");
	DEBUG(1, TERM_GREEN "|\tCleaning up...         |");
	DEBUG(1, TERM_GREEN "|------------------------------|");
	renderer_free();
	SDL_Quit();

	return 0;
}
