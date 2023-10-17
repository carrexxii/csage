#include "config.h"
#include "gfx/font.h"
#include "camera.h"
#include "entities/entity.h"
#include "map.h"

void test_init()
{
	map_new((ivec3s){ 64, 64, 32 });

	Entity e1 = entity_new((vec3s){ 0.0, 0.0, 0.0 }, MODEL_PATH "dwarf.glb");
}
