#include <vulkan/vulkan.h>

#include "taskmgr.h"
#include "input.h"
#include "camera.h"
#include "gfx/renderer.h"
#include "gfx/ui/ui.h"
#include "gfx/model.h"
#include "gfx/particles.h"
#include "maths/scratch.h"
#include "entities/entity.h"
#include "map.h"
#include "scenemgr.h"

#include "test.h"

static void switch_scene(enum SceneType type);
static void register_global_keys(void);
static void load_game(void);
static void load_scratch(void);

static enum SceneType curr_scene;
static struct Camera* curr_cam;
static struct Camera game_cam;
static struct Camera scratch_cam;

void scenemgr_init()
{
	scratch_cam = camera_new(VEC3(0.0f, 0.0f, -1.0f), VEC3(0.0f, -1.0f, 0.0f));
	camera_set_persp(&scratch_cam, global_config.winw, global_config.winh, glm_rad(69.0f));
	game_cam = camera_new(VEC3(0.0f, 0.0f, -1.0f), VEC3(0.0f, -1.0f, 0.0f));
	camera_set_persp(&game_cam, global_config.winw, global_config.winh, glm_rad(69.0f));

	VkRenderPass renderpass = renderer_init();
	scratch_init(renderpass);
	entities_init();
	test_init();
	
	ui_init(renderpass);
	font_init(renderpass);
	particles_init(renderpass);
	map_init(renderpass, &game_cam);
	models_init(renderpass);

	ui_build();

	taskmgr_init();

	global_light.ambient[0] = 1.0f;
	global_light.ambient[1] = 1.0f;
	global_light.ambient[2] = 1.0f;
	global_light.ambient[3] = 0.1f;
	glm_vec3_normalize(global_light.ambient);

	global_light.pos[0] = 10.0f;
	global_light.pos[1] = 0.0f;
	global_light.pos[2] = 20.0f;
	global_light.pos[3] = 0.5f;
	global_light.colour[0] = 1.0f;
	global_light.colour[1] = 1.0f;
	global_light.colour[2] = 1.0f;
	glm_vec3_normalize(global_light.colour);

	switch_scene(SCENE_SCRATCH);
}

noreturn void scenemgr_loop()
{
	DEBUG(1, "\nBeginning main loop (load time: %lums)\n"
	           "--------------------------------------", SDL_GetTicks64());
	uint64 dt, nt, ot = 0.0, acc = 0.0;
	while (1) {
		input_poll();
		nt = SDL_GetTicks64();
		dt = nt - ot;
		ot = nt;
		acc += dt;
		while (acc >= DT_MS) {
			// static float dir = 1.0;
			// global_light.pos[0] += dir;
			// if (fabs(global_light.pos[0]) > 50.0)
			// 	dir = -dir;

			while (!taskmgr_reset());
			acc -= DT_MS;
		}
		renderer_draw(curr_cam);
	}
}

/* -------------------------------------------------------------------- */

static void switch_scene(enum SceneType scene)
{
	if (scene != curr_scene) {
		taskmgr_clear();
		curr_scene = scene;
		switch (scene) {
		case SCENE_GAME   : load_game();    break;
		case SCENE_SCRATCH: load_scratch(); break;
		default:
			ERROR("Scene %d does not exist (curr_scene = %d)", scene, curr_scene); // TODO: STRING_OF_SCENE
			exit(1);
		}
	}
}

static void register_global_keys()
{
	input_reset();
	input_register(SDLK_ESCAPE, LAMBDA(void, bool kdown, if (kdown) quit();));
	input_register(SDLK_1, LAMBDA(void, bool kdown, if (kdown) switch_scene(SCENE_GAME);));
	input_register(SDLK_2, LAMBDA(void, bool kdown, if (kdown) switch_scene(SCENE_SCRATCH);));
}

static void load_game()
{
	curr_cam = &game_cam;

	register_global_keys();
	input_register(SDL_BUTTON_LEFT, map_mouse_select);
	input_register(SDL_BUTTON_RIGHT, map_mouse_deselect);
	input_register(SDLK_w, LAMBDA(void, bool kdown, camera_move(&game_cam, DIR_UP   , kdown);));
	input_register(SDLK_a, LAMBDA(void, bool kdown, camera_move(&game_cam, DIR_LEFT , kdown);));
	input_register(SDLK_s, LAMBDA(void, bool kdown, camera_move(&game_cam, DIR_DOWN , kdown);));
	input_register(SDLK_d, LAMBDA(void, bool kdown, camera_move(&game_cam, DIR_RIGHT, kdown);));

	renderer_clear_draw_list();
	renderer_add_to_draw_list(map_record_commands);
	renderer_add_to_draw_list(models_record_commands);
	renderer_add_to_draw_list(particles_record_commands);
	renderer_add_to_draw_list(ui_record_commands);
	renderer_add_to_draw_list(font_record_commands);

	taskmgr_add_task(ui_update);
	taskmgr_add_task(map_update);
	taskmgr_add_task(particles_update);
	taskmgr_add_task(entities_update);
	taskmgr_add_task(models_update); // TODO: Change the animation to a separate thing
	taskmgr_add_task(LAMBDA(void, void, camera_update(&game_cam);));
}

static void load_scratch()
{
	curr_cam = &scratch_cam;

	register_global_keys();
	input_register(SDLK_w, LAMBDA(void, bool kdown, camera_move(&scratch_cam, DIR_UP       , kdown);));
	input_register(SDLK_a, LAMBDA(void, bool kdown, camera_move(&scratch_cam, DIR_LEFT     , kdown);));
	input_register(SDLK_s, LAMBDA(void, bool kdown, camera_move(&scratch_cam, DIR_DOWN     , kdown);));
	input_register(SDLK_d, LAMBDA(void, bool kdown, camera_move(&scratch_cam, DIR_RIGHT    , kdown);));
	input_register(SDLK_q, LAMBDA(void, bool kdown, camera_move(&scratch_cam, DIR_FORWARDS , kdown);));
	input_register(SDLK_e, LAMBDA(void, bool kdown, camera_move(&scratch_cam, DIR_BACKWARDS, kdown);));
	scratch_load();

	renderer_clear_draw_list();
	renderer_add_to_draw_list(scratch_record_commands);

	taskmgr_add_task(LAMBDA(void, void, camera_update(&scratch_cam);));
}
