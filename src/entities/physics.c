#include "entity.h"
#include "components.h"
#include "systems.h"

void set_entity_pos(Entity e, vec3 pos)
{
	if (!(entities[e] & COMPONENT_MODEL))
		ERROR("[ENT] Entity %lu has no model component", e);

	mat4* mat = iarr_get(components.mats, e);
	mat_translate(mat, pos);
}

void apply_forces(Entity e)
{
	struct Body* body;
	for (uint i = 0; i < components.bodies.itemc; i++) {
		body = &((struct Body*)components.bodies.data)[i];

		/* Gravity */
		vec_add_ip(&body->forces[body->forcec], VEC3(0.0, 0.0, G));
		body->forcec = 1;
	}
}

void integrate_bodies()
{
	struct Body* body;
	for (uint i = 0; i < components.bodies.itemc; i++) {
		body = &((struct Body*)components.bodies.data)[i];

		// /* vel += 0.5 * acc * dt */
		vec_add_ip(&body->vel, vec_scale(body->acc, 0.5*dt));

		// /* pos += vel * dt */
		vec_add_ip(&body->pos, vec_scale(body->vel, dt));

		// /* Apply forces */
		for (uint j = 0; j < body->forcec; j++)
			vec_add_ip(&body->acc, vec_scale(body->forces[j], body->mass));
		vec_clamp(&body->acc, MAX_ACC);

		// /* vel += 0.5 * acc * dt */
		vec_add_ip(&body->vel, vec_scale(body->acc, 0.5*dt));
		vec_clamp(&body->vel, MAX_VEL);

		vec_print(body->forces[0]);
		vec_print(body->pos);
		DEBUG(1, "\n");
	}
}
