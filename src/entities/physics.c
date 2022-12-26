#include "entity.h"
#include "components.h"
#include "systems.h"

void entity_set_pos(Entity e, vec3 pos)
{
	if (!(entities[e] & COMPONENT_MODEL))
		ERROR("[ENT] Entity %lu has no model component", e);

	mat4* mat = iarr_get(components.mats, e);
	mat_translate(mat, pos);
}

void physics_integrate()
{
	vec3 newpos, acc;
	struct Body* body;
	for (uint i = 0; i < components.bodies.itemc; i++) {
		body = &((struct Body*)components.bodies.data)[i];
		acc  = VEC3(0.0, 0.0, G);
		vec_scale_ip(&acc, dt*dt);

		// x(t+1) = 2x(t) − x(t−1) + a(t)t^2
		newpos = vec_scale(body->pos, 2.0);
		vec_sub_ip(&newpos, body->prevpos);
		vec_add_ip(&newpos, acc);
		
		body->prevpos = body->pos;
		body->pos     = newpos;
	}
}

void physics_resolve_collisions()
{
	struct Body* body;
	for (uint i = 0; i < components.bodies.itemc; i++) {
		body = &((struct Body*)components.bodies.data)[i];
		// CLAMP(body->pos.z, -8.0, 8.0);
		if (collisions_map(body))
			body->pos.z = body->prevpos.z;
	}
}
