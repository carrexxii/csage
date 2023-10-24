#include "config.h"
#include "gfx/font.h"
#include "camera.h"
#include "entities/entity.h"
#include "entities/pathfinding.h"
#include "input.h"
#include "map.h"

static bool path_to_mouse(int type, bool kdown, int x, int y)
{
	vec2s screen = camera_get_map_point(camera_get_mouse_ray(x, y));
	DEBUG_VALUE(screen.x);
	DEBUG_VALUE(screen.y);
	DEBUG(1, "\n");
	path_new((ivec3s){ 0, 0, 0 }, (ivec3s){ screen.x, screen.y, 0 });

	return false;
}

void test_init()
{
	map_new((ivec3s){ 64, 64, MAP_BLOCK_DEPTH });

	Entity e1 = entity_new((vec3s){ 0.0, 0.0, 0.0 }, MODEL_PATH "dwarf.glb");

	input_register_mouse(MOUSE_DRAG, path_to_mouse);
}
