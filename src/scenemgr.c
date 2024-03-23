#include "vulkan/vulkan.h"

#include "taskmgr.h"
#include "maths/maths.h"
#include "gfx/renderer.h"
#include "gfx/ui/ui.h"
#include "gfx/model.h"
#include "gfx/particles.h"
#include "input.h"
#include "camera.h"
#include "entities/entity.h"
#include "entities/player.h"
#include "map.h"
#include "scenemgr.h"

#include "test.h"
#include "gfx/sprite.h"

#define MAX_DEFER_FUNCTIONS 32

typedef void (*DeferFn)(void*);

struct SceneDefer {
	DeferFn fn;
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

struct SpriteGroup* sprite;

void scenemgr_init()
{
	game_cam = camera_new(VEC3_ZERO, VEC3_ZERO, config.winw, config.winh, 0.0f);
	scenemgr_defer((DeferFn)camera_free, &game_cam);

	renderer_init();
	maps_init();
	sprites_init();
	font_init();
	ui_init();
	particles_init();

	test_init();

	ui_build();

	taskmgr_init();

	renderer_set_global_lighting(VEC3(0.2f , 0.2f , 1.0f),
	                             VEC3(0.01f, 0.01f, 0.01f),
	                             VEC3(0.01f, 0.01f, 0.01f),
	                             VEC3(0.2f , 0.2f , 0.2f));
	curr_cam = &game_cam;
	global_camera_ubo = game_cam.ubo;

	map_new(&map, "test");
	scenemgr_defer((DeferFn)map_free, &map);

	player_init();
	game_cam.follow = &player_sprite->pos;
	entities_init();

	switch_scene(SCENE_GAME);
}

noreturn void scenemgr_loop()
{
	DEBUG(1, "\nBeginning main loop (load time: %lums)\n"
	           "--------------------------------------", SDL_GetTicks());
	uint64 dt, nt, ot = 0, acc = 0;
	while (1) {
		input_poll();
		nt = SDL_GetTicks();
		dt = nt - ot;
		ot = nt;
		acc += dt;
		while (acc >= DT_MS) {
			while (!taskmgr_reset());
			acc -= DT_MS;
		}
		renderer_draw(curr_cam);
	}
}

void scenemgr_defer(DeferFn fn, void* data)
{
	defers[deferc++] = (struct SceneDefer){
		.fn   = fn,
		.data = data,
	};
}

void scenemgr_free()
{
	scene_exec_defer();
	sprite_sheet_free();
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
		if (curr_scene)
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
static void cb_game_move_up(bool kdown)        { player_set_moving(DIR_N, kdown); camera_move(&game_cam, DIR_UP   , kdown); }
static void cb_game_move_left(bool kdown)      { player_set_moving(DIR_W, kdown); camera_move(&game_cam, DIR_LEFT , kdown); }
static void cb_game_move_down(bool kdown)      { player_set_moving(DIR_S, kdown); camera_move(&game_cam, DIR_DOWN , kdown); }
static void cb_game_move_right(bool kdown)     { player_set_moving(DIR_E, kdown); camera_move(&game_cam, DIR_RIGHT, kdown); }
static void cb_game_move_forwards(bool kdown)  { game_cam.pos.z += 1.0; camera_move(&game_cam, DIR_FORWARDS , kdown); }
static void cb_game_move_backwards(bool kdown) { game_cam.pos.z -= 1.0; camera_move(&game_cam, DIR_BACKWARDS, kdown); }
static void cb_game_cam_update() { camera_update(&game_cam); }
static void cb_game_sprites_record(VkCommandBuffer cmd_buf, struct Camera*) {
	sprites_record_commands(cmd_buf);
}
static void load_game()
{
	curr_cam = &game_cam;
	global_camera_ubo = game_cam.ubo;

	register_global_keys();
	input_register(SDLK_w, cb_game_move_up);
	input_register(SDLK_a, cb_game_move_left);
	input_register(SDLK_s, cb_game_move_down);
	input_register(SDLK_d, cb_game_move_right);
	input_register(SDLK_q, cb_game_move_forwards);
	input_register(SDLK_e, cb_game_move_backwards);

	renderer_clear_draw_list();
	renderer_add_to_draw_list(cb_game_sprites_record);
	renderer_add_to_draw_list(ui_record_commands);
	renderer_add_to_draw_list(particles_record_commands);
	renderer_add_to_draw_list(font_record_commands);

	taskmgr_add_task(ui_update);
	taskmgr_add_task(particles_update);
	taskmgr_add_task(entities_update);
	taskmgr_add_task(cb_game_cam_update);
	taskmgr_add_task(sprites_update);
	taskmgr_add_task(player_update);
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
