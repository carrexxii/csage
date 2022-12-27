#include "map/map.h"
#include "entity.h"

bool collisions_map(struct Body* body)
{
	uint mapx = (uint)body->pos.x;
	uint mapy = (uint)body->pos.y;
	uint mapz = (uint)(body->pos.z + body->dim.z);
	struct MapCell cellbelow, cellleft, cellright, cellfwd, cellback;
	/* Map bounds checking */
	if ((body->pos.z < -body->dim.z || body->pos.z > map->dim.h) ||
		(body->pos.x < -body->dim.w || body->pos.x > map->dim.w) ||
		(body->pos.y < -body->dim.h || body->pos.y > map->dim.h))
		return false;

	cellbelow = map->data[map_get_block_index(mapx, mapy, mapz)];
	if (cellbelow.data)
		return true;

	/* TODO: other sides */

	return false;
}
