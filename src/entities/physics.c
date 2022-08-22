#include "cglm/cglm.h"

#include "entity.h"
#include "components.h"
#include "systems.h"

void set_entity_pos(Entity e, vec3 pos)
{
	if (!(entities[e] & COMPONENT_MODEL))
		ERROR("[ENT] Entity %lu has not model component", e);

	// mat4* mat = iarr_get(components.mats, e);
	// glm_translate(*mat, pos);

	float* mat = iarr_get(components.mats, e);
	mat[12] = pos[0];
	mat[13] = pos[1];
	mat[14] = pos[2];
}

void apply_forces()
{
	struct Body* body;
	for (uint i = 0; i < components.bodies.itemc; i++) {
		body = &((struct Body*)components.bodies.data)[i];

		/* Gravity */
		glm_vec3_add(body->forces[body->forcec].xyz, (float[]){ 0.0, 0.0, G }, body->forces[body->forcec].xyz);
		body->forcec = 1;
	}
}

void integrate_bodies()
{
	struct Body* body;
	for (uint i = 0; i < components.bodies.itemc; i++) {
		body = &((struct Body*)components.bodies.data)[i];

		/* vel += 0.5 * acc * dt */
		glm_vec3_muladds(body->acc.xyz, 0.5 * dt, body->vel.xyz);

		/* pos += vel * dt */
		glm_vec3_muladds(body->vel.xyz, dt, body->pos.xyz);

		/* Apply forces */
		for (uint j = 0; j < body->forcec; j++)
			glm_vec3_muladds(body->forces[j].xyz, body->mass, body->acc.xyz);

		/* vel += 0.5 * acc * dt */
		glm_vec3_muladds(body->acc.xyz, 0.5 * dt, body->vel.xyz);

		glm_vec3_print(body->forces[0].xyz, stderr);
		glm_vec3_print(body->pos.xyz, stderr);
		DEBUG(1, "\n");
	}
}
