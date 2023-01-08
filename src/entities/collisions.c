#include "map/map.h"
#include "entity.h"

bool collision_map(struct Body* body)
{
	int mapx = body->pos[0];
	int mapy = body->pos[1];
	int mapz = body->pos[2];
	struct MapCell cellbelow, cellleft, cellright, cellfwd, cellback;
	/* Map bounds checking */
	if ((body->pos[0] < -body->r/2.0 || body->pos[0] > map.w) ||
		(body->pos[1] < -body->r/2.0 || body->pos[1] > map.h) ||
		(body->pos[2] < 0.0 || body->pos[2] > map.d))
		return false;

	if (map_is_cell(mapx, mapy, mapz))
		return true;

	/* TODO: other sides */

	return false;
}

bool collision_ray(struct Body* body, struct Ray ray)
{
	struct Capsule cap = {
		.p1 = { body->pos[0], body->pos[1], body->pos[2]*CAPSULE_RADIUS_PERCENTAGE },
		.p2 = { body->pos[0], body->pos[1], body->pos[2] - body->pos[2]*CAPSULE_RADIUS_PERCENTAGE },
		.r  = body->r,
	};

	DEBUG(1, "p1: %.2f %.2f %.2f; p2: %.2f %.2f %.2f -> %f", cap.p1[0], cap.p1[1], cap.p1[2], cap.p2[0], cap.p2[1], cap.p2[2], cap.r);
	glm_vec3_print(ray.p, stderr);
	glm_vec3_print(ray.v, stderr);
	DEBUG(1, " - - - - - - - - - - - - - -\n");
	return false;
}
