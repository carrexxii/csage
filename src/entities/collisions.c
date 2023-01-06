#include "map/map.h"
#include "entity.h"

bool collisions_map(struct Body* body)
{
	int mapx = body->pos[0];
	int mapy = body->pos[1];
	int mapz = body->pos[2];
	struct MapCell cellbelow, cellleft, cellright, cellfwd, cellback;
	/* Map bounds checking */
	if ((body->pos[0] < -body->dim[0] || body->pos[0] > map.w) ||
		(body->pos[1] < -body->dim[1] || body->pos[1] > map.h) ||
		(body->pos[2] < -body->dim[2] || body->pos[2] > map.d))
		return false;

	if (map_is_cell(mapx, mapy, mapz))
		return true;

	/* TODO: other sides */

	return false;
}

