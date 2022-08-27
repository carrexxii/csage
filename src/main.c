#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include "cglm/cglm.h"

#include "config.h"
#include "taskmgr.h"
#include "gfx/vulkan.h"
#include "gfx/renderer.h"
#include "camera.h"
#include "input.h"
#include "entities/entity.h"

#include "gfx/model.h"
#include "entities/components.h"
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

	srand(SDL_GetTicks64());

	init_taskmgr();
	add_taskmgr_task(update_camera);
	add_taskmgr_task(update_entities);

	Entity e1 = create_entity();
	add_component(e1, COMPONENT_MODEL, MODEL_PATH "sphere");
	struct Body body = (struct Body){
		.dim    = (Vec3){ 1.0, 1.0,  1.0 },
		.pos    = (Vec3){ 5.0, 5.0, -5.0 },
		.maxVel = (Vec3){ 1.0, 1.0,  1.0 },
		.mass   = 1.0,
	};
	add_component(e1, COMPONENT_BODY, &body);

	// Entity e2 = create_entity();
	// add_component(e2, COMPONENT_MODEL, MODEL_PATH "plane");
	// set_entity_pos(e2, (vec3){ -0.5, 0.0, 0.0 });

	Entity e3 = create_entity();
	add_component(e3, COMPONENT_MODEL, MODEL_PATH "sphere");
	set_entity_pos(e3, (vec3){ 0.0, -20.0, 5.0 });
	add_component(e3, COMPONENT_LIGHT, (vec4){ -2.0, -20.0, 0.0, 0.07 });

	init_map(MAPTYPE_FILLED, (struct Dim){ .w=16, .h=16, .d=4, });

	DEBUG(1, "\nBeginning main loop (load time: %lums)\n"
	           "--------------------------------------", SDL_GetTicks64());
	uint64 delta, newtime, oldtime = 0.0, accum = 0.0;
	while (!check_input()) {
		newtime = SDL_GetTicks64();
		delta   = newtime - oldtime;
		oldtime = newtime;
		accum  += delta;
		while (accum >= dt) {
			// DEBUG_VALUE(delta);
			while (!reset_taskmgr());
			accum -= dt;
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
	register_key((struct InputCallback){ .key = SDLK_d, .fn = move_cam_right_cb, .onkeydown = true, .onkeyup = true, });
	register_key((struct InputCallback){ .key = SDLK_a, .fn = move_cam_left_cb , .onkeydown = true, .onkeyup = true, });
	register_key((struct InputCallback){ .key = SDLK_w, .fn = move_cam_up_cb   , .onkeydown = true, .onkeyup = true, });
	register_key((struct InputCallback){ .key = SDLK_s, .fn = move_cam_down_cb , .onkeydown = true, .onkeyup = true, });
	register_key((struct InputCallback){ .key = SDLK_q, .fn = zoom_cam_in_cb   , .onkeydown = true, .onkeyup = true, });
	register_key((struct InputCallback){ .key = SDLK_e, .fn = zoom_cam_out_cb  , .onkeydown = true, .onkeyup = true, });
	register_key((struct InputCallback){ .key = SDLK_o, .fn = cam_zlvl_up_cb   , .onkeydown = true, });
	register_key((struct InputCallback){ .key = SDLK_p, .fn = cam_zlvl_down_cb , .onkeydown = true, });
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
