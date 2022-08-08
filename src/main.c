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
#include "entities/entity.h"

#include "gfx/model.h"
#include "entities/systems.h"
#include "map/map.h"

void init_sdl();
void init_input();
noreturn void quit_cb(bool kdown);

SDL_Renderer* renderer;
SDL_Window*   window;

int main(int argc, char** argv)
{
	for (int i = 0; i < argc; i++)
		printf("%s\n", argv[i]);

#ifdef RUN_TESTS
	DEBUG(1, TERM_GREEN "---------------------\n--- Running Tests ---\n---------------------" TERM_NORMAL);
	test_arraylist();

	return 0;
#endif

	init_sdl();

	uint32 vkversion;
	vkEnumerateInstanceVersion(&vkversion);
	DEBUG(1, "[INFO] Vulkan version: %u.%u.%u", VK_API_VERSION_MAJOR(vkversion),
	      VK_API_VERSION_MINOR(vkversion), VK_API_VERSION_PATCH(vkversion));

	init_input();
	init_vulkan(window);
	renderer_init();
	init_cam();
	init_entities();

	Entity e1 = create_entity();
	add_component(e1, COMPONENT_MODEL, MODEL_PATH "sphere");
	set_entity_pos(e1, (vec3){ 5.0, 5.0, 0.0 });

	// Entity e2 = create_entity();
	// add_component(e2, COMPONENT_MODEL, MODEL_PATH "plane");
	// set_entity_pos(e2, (vec3){ -0.5, 0.0, 0.0 });

	Entity e3 = create_entity();
	add_component(e3, COMPONENT_MODEL, MODEL_PATH "sphere");
	set_entity_pos(e3, (vec3){ -2.0, -20.0, 0.0 });
	add_component(e3, COMPONENT_LIGHT, (vec4){ -2.0, -20.0, 0.0, 0.01 });

	init_map(MAPTYPE_FILLED, (struct Dim){ .w=64, .h=64, .d=64, });

	DEBUG(1, "\nBeginning main loop (load time: %lums)\n"
	           "--------------------------------------", SDL_GetTicks64());
	double dt, newtime, oldtime = 0.0, accum = 0.0;
	while (!check_input()) {
		newtime = SDL_GetTicks64();
		dt      = newtime - oldtime;
		oldtime = newtime;
		accum  += dt;
		while (accum >= FRAME_DT) {
			accum -= FRAME_DT;
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
	DEBUG(1, "/------------------------------\\");
	DEBUG(1, "|\tCleaning up...         |");
	DEBUG(1, "\\------------------------------/");
	renderer_free();
	free_map();
	free_entities();
	free_vulkan();
	SDL_Quit();
	exit(0);
}
