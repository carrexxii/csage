#include "entity.h"
#include "components.h"
#include "systems.h"

void physics_integrate()
{
	vec3 newpos, acc;
	struct Body* body;
	for (int i = 0; i < components.bodies.itemc; i++) {
		body = &((struct Body*)components.bodies.data)[i];
		glm_vec3_copy((vec3){ 0.0, 0.0, G }, acc);
		for (int j = 0; j < body->forcec; j++)
			glm_vec3_add(acc, body->forces[j], acc);
		glm_vec3_scale(acc, dt*dt, acc);

		// x(t+1) = 2x(t) − x(t−1) + a(t)t^2
		glm_vec3_scale(body->pos, 2.0, newpos);
		glm_vec3_sub(newpos, body->prevpos, newpos);
		glm_vec3_add(newpos, acc, newpos);
		
		glm_vec3_copy(body->pos, body->prevpos);
		glm_vec3_copy(newpos, body->pos);
		body->forcec  = 0;
	}
}

void physics_resolve_collisions()
{
	struct Body* body;
	for (int i = 0; i < components.bodies.itemc; i++) {
		body = &((struct Body*)components.bodies.data)[i];
		if (collisions_map(body))
			body->pos[2] = body->prevpos[2];
	}
}
