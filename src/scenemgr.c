#include "common.h"
#include "vulkan/vulkan.h"

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

#define MAX_DEFER_FUNCTIONS 32

struct SceneDefer {
	void (*fn)(void*);
	void* data;
};

static void scene_exec_defer(void);
static void switch_scene(enum SceneType type);
static void register_global_keys(void);
static void load_game(void);
static void load_scratch(void);

static enum SceneType curr_scene;
static struct Camera* curr_cam;
static struct Camera game_cam;
static struct Camera scratch_cam;

static isize deferc;
static struct SceneDefer defers[MAX_DEFER_FUNCTIONS];

void scenemgr_init()
{
	scratch_cam = camera_new(VEC3(0.0f, 0.0f, -1.0f), VEC3(0.0f, -1.0f, 0.0f), global_config.winw, global_config.winh, deg_to_rad(69.0f));
	game_cam    = camera_new(VEC3(-5.0f, 2.0f, -5.0f), VEC3(0.0f, -0.1f, 0.0f), global_config.winw, global_config.winh, deg_to_rad(45.0f));
	camera_set_projection(&scratch_cam, CAMERA_PERSPECTIVE);
	camera_set_projection(&game_cam, CAMERA_ORTHOGONAL);

	VkRenderPass renderpass = renderer_init();
	maps_init(renderpass);
	// scratch_init(renderpass);
	// font_init(renderpass);
	// ui_init(renderpass);
	// particles_init(renderpass);
	entities_init();

	test_init();
	models_init(renderpass);

	// ui_build();

	taskmgr_init();

	global_light.pos     = VEC4(10.0f, 0.0f, 20.0f, 0.5f);
	global_light.ambient = normalized(VEC4(1.0f, 1.0f, 1.0f, 0.1f));
	global_light.colour  = normalized(VEC3(1.0f, 1.0f, 1.0f));

	switch_scene(SCENE_GAME);
}

noreturn void scenemgr_loop()
{
	DEBUG(1, "\nBeginning main loop (load time: %lums)\n"
	           "--------------------------------------", SDL_GetTicks());
	uint64 dt, nt, ot = 0.0, acc = 0.0;
	while (1) {
		input_poll();
		nt = SDL_GetTicks();
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

void scenemgr_defer(void (*fn)(void*), void* data)
{
	defers[deferc++] = (struct SceneDefer){
		.fn   = fn,
		.data = data,
	};
}

void scenemgr_free()
{
	scene_exec_defer();
}

/* -------------------------------------------------------------------- */

static void scene_exec_defer()
{
	struct SceneDefer defer;
	while (deferc) {
		defer = defers[--deferc];
		defer.fn(defer.data);
	}
}

static void switch_scene(enum SceneType scene)
{
	if (scene != curr_scene) {
		scene_exec_defer();

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

static noreturn void cb_quit(bool) { quit(); }
static void cb_switch_scene_game(bool kdown)    { if (kdown) switch_scene(SCENE_GAME);    }
static void cb_switch_scene_scratch(bool kdown) { if (kdown) switch_scene(SCENE_SCRATCH); }
static void cb_toggle_view(bool kdown) {
	if (!kdown)
		return;
	struct Camera* cam = curr_scene == SCENE_GAME   ? &game_cam   :
	                     curr_scene == SCENE_SCRATCH? &scratch_cam: NULL;
	camera_set_projection(cam, cam->type == CAMERA_PERSPECTIVE? CAMERA_ORTHOGONAL: CAMERA_PERSPECTIVE);
}
static void register_global_keys()
{
	input_reset();
	input_register(SDLK_ESCAPE, cb_quit);
	input_register(SDLK_F1, cb_switch_scene_game);
	input_register(SDLK_F2, cb_switch_scene_scratch);

	input_register(SDLK_F12, cb_toggle_view);
}

static struct Map map;
static void cb_game_move_up(bool kdown)        { camera_move(&game_cam, DIR_UP       , kdown); }
static void cb_game_move_left(bool kdown)      { camera_move(&game_cam, DIR_LEFT     , kdown); }
static void cb_game_move_down(bool kdown)      { camera_move(&game_cam, DIR_DOWN     , kdown); }
static void cb_game_move_right(bool kdown)     { camera_move(&game_cam, DIR_RIGHT    , kdown); }
static void cb_game_move_forwards(bool kdown)  { camera_move(&game_cam, DIR_FORWARDS , kdown); }
static void cb_game_move_backwards(bool kdown) { camera_move(&game_cam, DIR_BACKWARDS, kdown); }
static void cb_game_cam_update() { camera_update(&game_cam); }
static void cb_game_map_record(VkCommandBuffer cmd_buf, struct Camera *cam) {
	map_record_commands(cmd_buf, cam, &map);
}
static void load_game()
{
	curr_cam = &game_cam;

	map_new(&map, "test");
	scenemgr_defer((void (*)(void*))map_free, &map);

	register_global_keys();
	input_register(SDLK_w, cb_game_move_up);
	input_register(SDLK_a, cb_game_move_left);
	input_register(SDLK_s, cb_game_move_down);
	input_register(SDLK_d, cb_game_move_right);
	input_register(SDLK_q, cb_game_move_forwards);
	input_register(SDLK_e, cb_game_move_backwards);


	renderer_clear_draw_list();
	renderer_add_to_draw_list(models_record_commands);
	renderer_add_to_draw_list(cb_game_map_record);
	// renderer_add_to_draw_list(ui_record_commands);
	// renderer_add_to_draw_list(particles_record_commands);
	// renderer_add_to_draw_list(font_record_commands);

	taskmgr_add_task(models_update); // TODO: Change the animation to a separate thing
	// taskmgr_add_task(ui_update);
	// taskmgr_add_task(particles_update);
	// taskmgr_add_task(entities_update);
	taskmgr_add_task(cb_game_cam_update);
}

static void cb_scratch_move_up(bool kdown)        { camera_move(&scratch_cam, DIR_UP       , kdown); }
static void cb_scratch_move_left(bool kdown)      { camera_move(&scratch_cam, DIR_LEFT     , kdown); }
static void cb_scratch_move_down(bool kdown)      { camera_move(&scratch_cam, DIR_DOWN     , kdown); }
static void cb_scratch_move_right(bool kdown)     { camera_move(&scratch_cam, DIR_RIGHT    , kdown); }
static void cb_scratch_move_forwards(bool kdown)  { camera_move(&scratch_cam, DIR_FORWARDS , kdown); }
static void cb_scratch_move_backwards(bool kdown) { camera_move(&scratch_cam, DIR_BACKWARDS, kdown); }
static void cb_scratch_cam_update() { camera_update(&scratch_cam); }
static void load_scratch()
{
	curr_cam = &scratch_cam;

	register_global_keys();
	input_register(SDLK_w, cb_scratch_move_up);
	input_register(SDLK_a, cb_scratch_move_left);
	input_register(SDLK_s, cb_scratch_move_down);
	input_register(SDLK_d, cb_scratch_move_right);
	input_register(SDLK_q, cb_scratch_move_forwards);
	input_register(SDLK_e, cb_scratch_move_backwards);
	scratch_load();

	renderer_clear_draw_list();
	renderer_add_to_draw_list(scratch_record_commands);

	taskmgr_add_task(cb_scratch_cam_update);
}
