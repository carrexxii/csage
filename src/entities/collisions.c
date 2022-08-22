#include "map/map.h"
#include "entity.h"

void resolve_collisions()
{
	struct Body* body;
	for (uint i = 0; i < components.bodies.itemc; i++) {
		body = &((struct Body*)components.bodies.data)[i];
		body->forcec = 0;

	}
}
