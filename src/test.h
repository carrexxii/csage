#include "config.h"
#include "maths/maths.h"
#include "gfx/ui/font.h"
#include "camera.h"
#include "entities/entity.h"
#include "entities/pathfinding.h"
#include "util/arena.h"
#include "input.h"
#include "gfx/ui/ui.h"
#include "maths/scratch.h"
#include "map.h"

static EntityID ent;

static bool path_to_mouse(int type, bool kdown, int x, int y)
{
	(void)type;
	(void)kdown;
	// vec2s screen = camera_get_map_point(camera_get_mouse_ray(x, y));

	// struct Path path = {.start = (ivec3s){ 0, 0, 0 }, .end = (ivec3s){ screen.x, screen.y, 0 }};
	// path_new(&path);

	return false;
}

static bool move_to_mouse(int type, bool kdown, int x, int y)
{
	// (void)type;
	// if (kdown) {
		// vec2s screen = camera_get_map_point(camera_get_mouse_ray(x, y));
		// entity_path_to(e, (ivec3s){ screen.x, screen.y, 0 });
	// }

	return true;
}

static void test_scratch()
{
	// var A=point(0,.8,0), B=point(.8,-1,-.8), C=point(-.8,-1,-.8), D=point(.8,-1,.8), E=point(-.8,-1,.8);
	Trivec a = TRIVEC( 0.0f,  0.8f,  0.0f, 1.0f);
	Trivec b = TRIVEC( 0.8f, -1.0f, -0.8f, 1.0f);
	Trivec c = TRIVEC(-0.8f, -1.0f, -0.8f, 1.0f);
	Trivec d = TRIVEC( 0.8f, -1.0f,  0.8f, 1.0f);
	Trivec e = TRIVEC(-0.8f, -1.0f,  0.8f, 1.0f);

	/* Points */
	scratch_add(a);
	scratch_add(b);
	scratch_add(c);
	scratch_add(d);
	scratch_add(e);

	/* Lines between the points */
	scratch_add(join(a, b));
	scratch_add(join(a, c));
	scratch_add(join(a, d));
	scratch_add(join(a, e));
	scratch_add(join(b, c));
	scratch_add(join(e, c));
	scratch_add(join(b, d));
	scratch_add(join(d, e));

	scratch_add(join(join(a, b), c));
	Trivec base[4] = { b, c, e, d };
	scratch_add(base);

	// Vec v1 = VEC(1.0, 1.0, 1.0, 1.0);
	// Vec v2 = VEC(2.0, 2.0, 2.0, 1.0);
	// Vec v3 = VEC(0.0, 0.0, 0.0, 1.0);
	// Vec v4 = VEC(-5.0, 0.0, 1.0, 1.0);
	// Bivec b1 = wedge(v1, v2);
	// Bivec b2 = wedge(v3, v4);
	// scratch_add(b1);
	// scratch_add(b2);

	// Vec v = { 0.0, 0.0, 0.0, 1.0 };
	// scratch_add(v);
}

static struct Map map;
static void cb_test() { DEBUG(1, "Hello"); }
static void test_init()
{
	// ent = entity_new(VEC3(0.0, 0.0, 0.0), MODEL_PATH "tile.glb");

	// test_scratch();

	// struct UIStyle test_style = {
	// 	.bg = 0xFF00FFFF,
	// 	.fg = 0xDDDDDDFF,
	// };
	// struct UIObject* c1 = container_new(RECT(-1.0, -1.0, 1.0, 1.0), NULL, NULL);
	// button_new(STRING("Hello1"), RECT(1.0, 1.0, 100.0, 40.0), cb_test, NULL, c1);
	// button_new(STRING("Hello2"), RECT(-1.0, 1.0, 100.0, 40.0), cb_test, NULL, c1);
	// String str1 = STRING("Testing textbox where the text goes on for a long ass time until it needs to go over multiple lines Testing textbox where the text goes on for a long ass time until it needs to go over multiple lines Testing textbox where the text goes on for a long ass time until it needs to go over multiple lines Testing textbox where the text goes on for a long ass time until it needs to go over multiple linesTesting textbox where the text goes on for a long ass time until it needs to go over multiple lines Testing textbox where the text goes on for a long ass time until it needs to go over multiple lines Testing textbox where the text goes on for a long ass time until it needs to go over multiple lines Testing textbox where the text goes on for a long ass time until it needs to go over multiple lines");
	// textbox_new(str1, RECT(-0.8, -0.8, 1.6, 1.2), NULL, c1);
}

static void test_free()
{
	map_free(&map);
}
