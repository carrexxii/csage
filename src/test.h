#include "config.h"
#include "gfx/font.h"
#include "camera.h"
#include "entities/entity.h"
#include "entities/pathfinding.h"
#include "util/arena.h"
#include "input.h"
#include "map.h"
#include "gfx/ui/ui.h"

static EntityID e1;

static bool path_to_mouse(int type, bool kdown, int x, int y)
{
	(void)type;
	(void)kdown;
	vec2s screen = camera_get_map_point(camera_get_mouse_ray(x, y));

	struct Path path = {.start = (ivec3s){ 0, 0, 0 }, .end = (ivec3s){ screen.x, screen.y, 0 }};
	path_new(&path);

	return false;
}

static bool move_to_mouse(int type, bool kdown, int x, int y)
{
	(void)type;
	if (kdown) {
		vec2s screen = camera_get_map_point(camera_get_mouse_ray(x, y));
		entity_path_to(e1, (ivec3s){ screen.x, screen.y, 0 });
	}

	return true;
}

static void test_init()
{
	input_register(SDL_KEYDOWN, SDLK_ESCAPE, quit);

	map_new((ivec3s){ 64, 64, 16 });

	e1 = entity_new((vec3s){ 0.0, 0.0, 0.0 }, MODEL_PATH "cube.glb");

	struct UIStyle test_style = {
		.title  = NULL,
		.margin = 10,
		.bg = 0xFF00FFFF,
		.fg = 0xDDDDDDFF,
	};
	int c1 = container_new(RECT(0.0, 0.0, 0.5, 0.5), NULL, -1);
	// int c2 = container_new(RECT(0.5, 0.5, 0.5, 0.5), NULL, -1);
	// int c3 = container_new(RECT(0.5, 0.5, 0.5, 0.4), &test_style, c1);
	int b1 = button_new("Hello", RECT(0.0, 0.0, 100.0, 40.0), NULL, c1);
	ui_build();
}
