#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

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
	camera_init();
	entities_init();

	srand(SDL_GetTicks64());

	taskmgr_init();
	taskmgr_add_task(camera_update);
	taskmgr_add_task(entities_update);

	Entity e1 = entity_new();
	entity_add_component(e1, COMPONENT_MODEL, MODEL_PATH "sphere");
	struct Body body = (struct Body){
		.pos = { 1.0, 1.0, -5.0 },
		.dim = { 0.5, 0.5,  1.0 },
	};
	entity_add_component(e1, COMPONENT_BODY, &body);
	// entity_move(e1, VEC3(1.0, 0.0, 0.0));

	// Entity e2 = create_entity();
	// add_component(e2, COMPONENT_MODEL, MODEL_PATH "plane");
	// set_entity_pos(e2, (vec3){ -0.5, 0.0, 0.0 });

	// Entity e3 = create_entity();
	// add_component(e3, COMPONENT_MODEL, MODEL_PATH "sphere");
	// set_entity_pos(e3, VEC3(0.0, -20.0, 5.0));
	// add_component(e3, COMPONENT_LIGHT, VEC4(-2.0, -20.0, 0.0, 0.07).arr);

	map_init(MAPTYPE_FILLED, 16, 16, 4);

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
	input_register_key((struct InputCallback){ .key = SDLK_ESCAPE, .fn = quit_cb, .onkeydown = true });
	input_register_key((struct InputCallback){ .key = SDLK_d, .fn = camera_move_right_cb, .onkeydown = true, .onkeyup = true, });
	input_register_key((struct InputCallback){ .key = SDLK_a, .fn = camera_move_left_cb , .onkeydown = true, .onkeyup = true, });
	input_register_key((struct InputCallback){ .key = SDLK_w, .fn = camera_move_up_cb   , .onkeydown = true, .onkeyup = true, });
	input_register_key((struct InputCallback){ .key = SDLK_s, .fn = camera_move_down_cb , .onkeydown = true, .onkeyup = true, });
	input_register_key((struct InputCallback){ .key = SDLK_q, .fn = camera_zoom_in_cb   , .onkeydown = true, .onkeyup = true, });
	input_register_key((struct InputCallback){ .key = SDLK_e, .fn = camera_zoom_out_cb  , .onkeydown = true, .onkeyup = true, });
	input_register_key((struct InputCallback){ .key = SDLK_o, .fn = camera_zlvl_up_cb   , .onkeydown = true, });
	input_register_key((struct InputCallback){ .key = SDLK_p, .fn = camera_zlvl_down_cb , .onkeydown = true, });

	input_register_mouse(MOUSE_BUTTON_LEFT, map_select_cell);
}

noreturn void quit_cb(bool kdown)
{
	DEBUG(1, "/------------------------------\\");
	DEBUG(1, "|\tCleaning up...         |");
	DEBUG(1, "\\------------------------------/");
	renderer_free();
	map_free();
	entities_free();
	free_vulkan();
	SDL_Quit();
	exit(0);
}
