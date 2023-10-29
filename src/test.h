#include "config.h"
#include "gfx/font.h"
#include "camera.h"
#include "entities/entity.h"
#include "entities/pathfinding.h"
#include "input.h"
#include "map.h"

static EntityID e1;

static bool path_to_mouse(int type, bool kdown, int x, int y)
{
	vec2s screen = camera_get_map_point(camera_get_mouse_ray(x, y));

	struct Path path = {.start = (ivec3s){ 0, 0, 0 }, .end = (ivec3s){ screen.x, screen.y, 0 }};
	path_new(&path);

	return false;
}

static bool move_to_mouse(int type, bool kdown, int x, int y)
{
	if (kdown) {
		vec2s screen = camera_get_map_point(camera_get_mouse_ray(x, y));
		entity_path_to(e1, (ivec3s){ screen.x, screen.y, 0 });
	}

	return true;
}

void test_init()
{
	map_new((ivec3s){ 64, 64, 16 });

	e1 = entity_new((vec3s){ 0.0, 0.0, 0.0 }, MODEL_PATH "chicken.glb");
	DEBUG_VALUE(e1);

	// input_register_mouse(MOUSE_DRAG, path_to_mouse);
	input_register_mouse(MOUSE_RIGHT, move_to_mouse);
}
