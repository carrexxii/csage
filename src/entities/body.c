#include "component.h"
#include "body.h"

void bodies_update(struct ComponentArray* bodies)
{
	struct Body* body;
	uint16 e;
	FOREACH_COMPONENT(bodies, e, body,
		if (body->moving)
			body->pos.x += body->vel;
	);
}
