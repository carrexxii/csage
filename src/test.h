#include "config.h"
#include "gfx/ui/font.h"
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
	input_register(SDLK_ESCAPE, LAMBDA(void, bool kdown, if (kdown) quit();));

	map_new((ivec3s){ 64, 64, 16 });

	e1 = entity_new((vec3s){ 5.0, 5.0, -5.0 }, MODEL_PATH "RiggedFigure.glb");

	// struct UIStyle test_style = {
		// .bg = 0xFF00FFFF,
		// .fg = 0xDDDDDDFF,
	// };
	// struct UIObject* c1 = container_new(RECT(-1.0, -1.0, 1.0, 1.0), NULL, NULL);
	// button_new(STRING("Hello1"), RECT(1.0, 1.0, 100.0, 40.0), LAMBDAV(printf("Hello1\n");), NULL, c1);
	// button_new(STRING("Hello2"), RECT(-1.0, 1.0, 100.0, 40.0), LAMBDAV(printf("Hello2\n");), NULL, c1);
	// String str1 = STRING("Testing textbox where the text goes on for a long ass time until it needs to go over multiple lines Testing textbox where the text goes on for a long ass time until it needs to go over multiple lines Testing textbox where the text goes on for a long ass time until it needs to go over multiple lines Testing textbox where the text goes on for a long ass time until it needs to go over multiple linesTesting textbox where the text goes on for a long ass time until it needs to go over multiple lines Testing textbox where the text goes on for a long ass time until it needs to go over multiple lines Testing textbox where the text goes on for a long ass time until it needs to go over multiple lines Testing textbox where the text goes on for a long ass time until it needs to go over multiple lines");
	// textbox_new(str1, RECT(-0.8, -0.8, 1.6, 1.2), NULL, c1);
}
