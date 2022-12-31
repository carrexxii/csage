#include "map/map.h"
#include "entity.h"

bool collisions_map(struct Body* body)
{
	uint mapx = (uint)body->pos[0];
	uint mapy = (uint)body->pos[1];
	uint mapz = (uint)(body->pos[2] + body->dim[2]);
	struct MapCell cellbelow, cellleft, cellright, cellfwd, cellback;
	/* Map bounds checking */
	if ((body->pos[0] < -body->dim[0] || body->pos[0] > map->w) ||
		(body->pos[1] < -body->dim[1] || body->pos[1] > map->h) ||
		(body->pos[2] < -body->dim[2] || body->pos[2] > map->d))
		return false;

	cellbelow = map->data[map_get_block_index(mapx, mapy, mapz)];
	if (cellbelow.data)
		return true;

	/* TODO: other sides */

	return false;
}
