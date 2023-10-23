#include "config.h"
#include "gfx/font.h"
#include "camera.h"
#include "entities/entity.h"
#include "entities/pathfinding.h"
#include "map.h"

void test_init()
{
	map_new((ivec3s){ 64, 64, MAP_BLOCK_DEPTH });

	Entity e1 = entity_new((vec3s){ 2.0, 6.0, 0.0 }, MODEL_PATH "dwarf.glb");

	path_new((ivec3s){ 2, 6, 0 }, (ivec3s){ 10, 10, 0 });
}
