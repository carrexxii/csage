#include "util/iarray.h"
#include "components.h"
#include "gfx/renderer.h" // struct Model
#include "gfx/model.h"
#include "systems.h"
#include "entity.h"

intptr entityc;
Entity entities[MAX_ENTITIES];
struct EntityComponents components;

void entities_init()
{
	components.mats      = iarr_new(sizeof(mat4), 10);
	components.mdls      = iarr_new(sizeof(struct Model), 10);
	components.bodies    = iarr_new(sizeof(struct Body), 10);
	components.thrusters = iarr_new(sizeof(struct Thruster), 10);

	renmats = components.mats.data;
	renmdls = components.mdls.data;
	renmdlc = &entityc;

	DEBUG(1, "[ENT] Initialized entities");
}

Entity entity_new()
{
	entities[++entityc] = COMPONENT_NONE;
	return entityc;
}

void* entity_add_component(Entity e, enum Component c, void* data)
{
	void* obj = NULL;
	switch (c) {
		case COMPONENT_NONE:
			ERROR("[ENT] Trying to add COMPONENT_NONE");
			break;
		case COMPONENT_MODEL:
			struct Model mdl = *(struct Model*)data;
			obj = iarr_append(&components.mdls, e, &mdl);
			break;
		case COMPONENT_BODY:
			obj = iarr_append(&components.bodies, e, (struct Body*)data);
			break;
		case COMPONENT_THRUSTER:
			obj = iarr_append(&components.thrusters, e, (struct Thruster*)data);
			break;
		default:
			ERROR("[ENT] Unhandled component add: %u", c);
	}
	entities[e] |= c;
	return obj;
}

void entities_update()
{
	struct Thruster* thrusters = components.thrusters.data;
	for (int i = 0; i < components.thrusters.itemc; i++)
		if (components.thrusters.inds[i] && thrusters[i].F > FLT_EPSILON)
			physics_apply_thrust(thrusters[i]);

	struct Body* bodies = components.bodies.data;
	for (int i = 0; i < components.bodies.itemc; i++)
		if (components.bodies.inds[i])
			physics_integrate(bodies + i);

	/* Update the model matrix for each entity */
	mat4 mat;
	struct Body* body = components.bodies.data;
	for (int i = 0; i < components.bodies.itemc; body++, i++) {
		glm_mat4_identity(mat);
		// ********************************************************************
		     if (body->s[0] >  3.0) body->s[0] = -3.0;
		else if (body->s[0] < -3.0) body->s[0] =  3.0;
		else if (body->s[1] >  3.0) body->s[1] = -3.0;
		else if (body->s[1] < -3.0) body->s[1] =  3.0;
		// ********************************************************************
		glm_rotate(mat, body->θ, (vec3){ 0.0, 0.0, 1.0 });
		glm_translate(mat, (vec3){ body->s[0], body->s[1], 0.0 });
		glm_mat4_ucopy(mat, ((mat4*)components.mats.data)[i]);
	}
}

void entities_free()
{
	DEBUG(1, "[ENT] Freeing entities...");
	iarr_free(&components.mats, NULL);
	iarr_free(&components.mdls, (void (*)(void*))model_free);
	iarr_free(&components.bodies, NULL);
}
