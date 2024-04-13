#include <vulkan/vulkan.h>
#include "SDL3/SDL.h"

#include "taskmgr.h"
#include "maths/maths.h"
#include "maths/scratch.h"
#include "lua.h"
#include "gfx/renderer.h"
#include "gfx/ui/ui.h"
#include "gfx/model.h"
#include "gfx/particles.h"
#include "input.h"
#include "camera.h"
#include "entities/entity.h"
#include "entities/player.h"
#include "map.h"
#include "editor.h"
#include "scenemgr.h"

#include "resmgr.h"
#include "gfx/sprite.h"

#define MAX_DEFER_FUNCTIONS 32

typedef void (*DeferFn)(void*);

typedef struct SceneDefer {
	DeferFn fn;
	void* data;
} SceneDefer;

static void scene_exec_defer(void);
static void switch_scene(enum SceneType type);
static void register_global_keys(void);
static void load_level(char* name);
static void load_game(void);
static void load_editor(void);
static void load_scratch(void);

static SDL_Window* window;
static SceneType   curr_scene;
static Camera      game_cam;
static Camera      editor_cam;
static Camera      scratch_cam;

static isize      deferc;
static SceneDefer defers[MAX_DEFER_FUNCTIONS];

///
Map map;
///

void scenemgr_init(SDL_Window* win)
{
	window = win;
	taskmgr_init();

	game_cam   = camera_new(VEC3_ZERO, VEC3_ZERO, config.winw, config.winh, 0.0f, &global_camera_ubo);
	editor_cam = camera_new(VEC3_ZERO, VEC3_ZERO, config.winw, config.winh, 0.0f, &global_camera_ubo);

	renderer_init();
	pipelns_init();
	maps_init();
	sprites_init();
	font_init();
	ui_init();
	particles_init();
	entities_init();

	renderer_set_global_lighting(VEC3(0.2f , 0.2f , 1.0f),
	                             VEC3(0.01f, 0.01f, 0.01f),
	                             VEC3(0.01f, 0.01f, 0.01f),
	                             VEC3(0.2f , 0.2f , 0.2f));

	map_new(&map, "test");
	scenemgr_defer((DeferFn)map_free, &map);

	player_init();
	game_cam.follow = &entity_get_body(player_entity, player_group)->pos;

	load_level("test");
	switch_scene(SCENE_GAME);
}

[[noreturn]]
void scenemgr_loop()
{
	INFO(TERM_GRAY "\nBeginning main loop (load time: %lums)\n"
	           "--------------------------------------", SDL_GetTicks());
	char title[64];
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

		snprintf(title, 64, "%lums", dt);
		SDL_SetWindowTitle(window, title);

		renderer_draw();
	}
}

void scenemgr_defer(DeferFn fn, void* data)
{
	defers[deferc++] = (SceneDefer){
		.fn   = fn,
		.data = data,
	};
}

void scenemgr_free()
{
	scene_exec_defer();
	editor_free();
	sprite_sheets_free();

	camera_free(&game_cam);
	camera_free(&editor_cam);
}

/* -------------------------------------------------------------------- */

static void scene_exec_defer()
{
	SceneDefer defer;
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
		case SCENE_EDITOR : load_editor();  break;
		case SCENE_SCRATCH: load_scratch(); break;
		default:
			ERROR("Scene %d does not exist (curr_scene = %d)", scene, curr_scene); // TODO: STRING_OF_SCENE
			exit(1);
		}
	}
}

[[noreturn]]
static void cb_quit(bool) { quit(); }
static void cb_switch_scene_game(bool kdown)    { if (kdown) switch_scene(SCENE_GAME);    }
static void cb_switch_scene_editor(bool kdown)  { if (kdown) switch_scene(SCENE_EDITOR);  }
static void cb_switch_scene_scratch(bool kdown) { if (kdown) switch_scene(SCENE_SCRATCH); }
static void cb_toggle_view(bool kdown) {
	if (!kdown) return;
	Camera* cam = curr_scene == SCENE_GAME  ? &game_cam  :
	                     curr_scene == SCENE_EDITOR? &editor_cam:
	                     NULL;
}
static void register_global_keys()
{
	input_reset();
	input_register(SDLK_ESCAPE, cb_quit);
	input_register(SDLK_F1, cb_switch_scene_game);
	input_register(SDLK_F2, cb_switch_scene_editor);

	input_register(SDLK_F12, cb_toggle_view);

	ui_register_keys();
}

/* -------------------------------------------------------------------- */

static void load_level(char* name)
{
	char path[PATH_BUFFER_SIZE];
	snprintf(path, PATH_BUFFER_SIZE, LEVEL_PATH "/%s.lua", name);
	lua_getglobal(lua_state, "load_level");
	if (lua_pcall(lua_state, 0, 1, 0)) {
		ERROR("[LUA] Failed in call to \"load_level\": \n\t%s", lua_tostring(lua_state, -1));
		return;
	}
	const Level* lvl = lua_topointer(lua_state, -1);
	lua_pop(lua_state, 1);

	INFO(TERM_GRAY "[SCENE] Loaded level \"%s\"", lvl->name.data);
}

static void cb_game_move_up(bool kdown)        { player_set_moving(DIR_N, kdown); camera_move(&game_cam, DIR_UP   , kdown); }
static void cb_game_move_left(bool kdown)      { player_set_moving(DIR_W, kdown); camera_move(&game_cam, DIR_LEFT , kdown); }
static void cb_game_move_down(bool kdown)      { player_set_moving(DIR_S, kdown); camera_move(&game_cam, DIR_DOWN , kdown); }
static void cb_game_move_right(bool kdown)     { player_set_moving(DIR_E, kdown); camera_move(&game_cam, DIR_RIGHT, kdown); }
static void cb_game_move_forwards(bool kdown)  { game_cam.pos.z += 1.0; camera_move(&game_cam, DIR_FORWARDS , kdown); }
static void cb_game_move_backwards(bool kdown) { game_cam.pos.z -= 1.0; camera_move(&game_cam, DIR_BACKWARDS, kdown); }
static void cb_game_cam_update() { camera_update(&game_cam); }
static void load_game()
{
	register_global_keys();
	input_register(SDLK_w, cb_game_move_up);
	input_register(SDLK_a, cb_game_move_left);
	input_register(SDLK_s, cb_game_move_down);
	input_register(SDLK_d, cb_game_move_right);
	input_register(SDLK_q, cb_game_move_forwards);
	input_register(SDLK_e, cb_game_move_backwards);

	renderer_clear_draw_list();
	renderer_add_to_draw_list(sprites_record_commands);
	renderer_add_to_draw_list(ui_record_commands);
	renderer_add_to_draw_list(particles_record_commands);
	renderer_add_to_draw_list(font_record_commands);

	taskmgr_add_task(ui_update);
	taskmgr_add_task(particles_update);
	taskmgr_add_task(entities_update);
	taskmgr_add_task(cb_game_cam_update);
	taskmgr_add_task(sprites_update);
}

/* -------------------------------------------------------------------- */

static void cb_editor_player_move_up(bool kdown)    { player_set_moving(DIR_N, kdown); }
static void cb_editor_player_move_left(bool kdown)  { player_set_moving(DIR_W, kdown); }
static void cb_editor_player_move_down(bool kdown)  { player_set_moving(DIR_S, kdown); }
static void cb_editor_player_move_right(bool kdown) { player_set_moving(DIR_E, kdown); }
static void cb_editor_move_up(bool kdown)    { camera_move(&editor_cam, DIR_UP   , kdown); }
static void cb_editor_move_left(bool kdown)  { camera_move(&editor_cam, DIR_LEFT , kdown); }
static void cb_editor_move_down(bool kdown)  { camera_move(&editor_cam, DIR_DOWN , kdown); }
static void cb_editor_move_right(bool kdown) { camera_move(&editor_cam, DIR_RIGHT, kdown); }
static void cb_editor_move_forwards(bool kdown)  { camera_move(&editor_cam, DIR_FORWARDS , kdown); }
static void cb_editor_move_backwards(bool kdown) { camera_move(&editor_cam, DIR_BACKWARDS, kdown); }
static void cb_editor_cam_update() { camera_update(&editor_cam); }
static void cb_editor_update() { editor_update(&editor_cam); }
static void load_editor()
{
	if (!editor_has_init)
		editor_init();

	register_global_keys();
	editor_register_keys();
	input_register(SDLK_UP   , cb_editor_player_move_up);
	input_register(SDLK_LEFT , cb_editor_player_move_left);
	input_register(SDLK_DOWN , cb_editor_player_move_down);
	input_register(SDLK_RIGHT, cb_editor_player_move_right);
	input_register(SDLK_w, cb_editor_move_up);
	input_register(SDLK_a, cb_editor_move_left);
	input_register(SDLK_s, cb_editor_move_down);
	input_register(SDLK_d, cb_editor_move_right);
	input_register(SDLK_q, cb_editor_move_forwards);
	input_register(SDLK_e, cb_editor_move_backwards);

	renderer_clear_draw_list();
	renderer_add_to_draw_list(sprites_record_commands);
	renderer_add_to_draw_list(particles_record_commands);
	renderer_add_to_draw_list(ui_record_commands);
	renderer_add_to_draw_list(font_record_commands);

	taskmgr_add_task(ui_update);
	taskmgr_add_task(particles_update);
	taskmgr_add_task(entities_update);
	taskmgr_add_task(cb_editor_cam_update);
	taskmgr_add_task(sprites_update);
	taskmgr_add_task(cb_editor_update);

	ui_build();
}

/* -------------------------------------------------------------------- */

static void cb_scratch_move_up(bool kdown)        { camera_move(&scratch_cam, DIR_UP       , kdown); }
static void cb_scratch_move_left(bool kdown)      { camera_move(&scratch_cam, DIR_LEFT     , kdown); }
static void cb_scratch_move_down(bool kdown)      { camera_move(&scratch_cam, DIR_DOWN     , kdown); }
static void cb_scratch_move_right(bool kdown)     { camera_move(&scratch_cam, DIR_RIGHT    , kdown); }
static void cb_scratch_move_forwards(bool kdown)  { camera_move(&scratch_cam, DIR_FORWARDS , kdown); }
static void cb_scratch_move_backwards(bool kdown) { camera_move(&scratch_cam, DIR_BACKWARDS, kdown); }
static void cb_scratch_cam_update() { camera_update(&scratch_cam); }
static void load_scratch()
{
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

