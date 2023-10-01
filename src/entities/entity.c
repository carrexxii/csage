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
	components.mats   = iarr_new(sizeof(mat4), 10);
	components.mdls   = iarr_new(sizeof(struct Model), 10);
	components.bodies = iarr_new(sizeof(struct Body), 10);

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

void entity_add_component(Entity e, enum Component c, void* data)
{
	if (!e || (intptr)e > entityc)
		ERROR("[ENT] Invalid entity");
	switch (c) {
		case COMPONENT_NONE:
			ERROR("[ENT] Trying to add COMPONENT_NONE");
			break;
		case COMPONENT_MODEL:
			struct Model mdl = *(struct Model*)data;
			iarr_append(&components.mdls, e, &mdl);
			break;
		case COMPONENT_BODY:
			iarr_append(&components.bodies, e, (struct Body*)data);
			break;
		default:
			ERROR("[ENT] Unhandled component add: %u", c);
	}
	entities[e] |= c;
}

void entities_update()
{
	/* Update the model matrices for each entity */
	mat4 mat;
	struct Body body;
	void* arr;
	for (int i = 0; i < entityc; i++) {
		body = ((struct Body*)components.bodies.data)[i];
		glm_mat4_identity(mat);
		glm_translate(mat, (vec3){ body.pos[0], body.pos[1], 0.0 });
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
