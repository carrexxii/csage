#include "util/iarray.h"
#include "gfx/model.h"
#include "components.h"
#include "gfx/renderer.h"
#include "systems.h"
#include "entity.h"

uint64 entityc;
uint64 entities[MAX_ENTITIES];
struct EntityComponents components;

void entities_init()
{
	components.mdls   = iarr_new(sizeof(struct Model), 10);
	components.mats   = iarr_new(sizeof(mat4), 10);
	components.lights = iarr_new(sizeof(vec4), 5);
	components.bodies = iarr_new(sizeof(struct Body), 10);

	renmdlc = &components.mdls.itemc;
	renmdls = components.mdls.data;
	renmats = components.mats.data;

	DEBUG(1, "[ENT] Initialized entities");
}

Entity entity_new()
{
	entities[++entityc] = COMPONENT_NONE;
	return entityc;
}

void entity_add_component(Entity e, enum Component c, void* data)
{
	struct Model mdl;
	vec4* light;
	switch (c) {
		case COMPONENT_NONE:
			ERROR("[ENT] Trying to add COMPONENT_NONE");
			break;
		case COMPONENT_MODEL:
			mdl = create_model((char*)data);
			iarr_append(&components.mdls, e, &mdl);
			iarr_append(&components.mats, e, &MAT_I4);
			break;
		case COMPONENT_LIGHT:
			light = iarr_append(&components.lights, e, data);
			renderer_add_light(*light);
			break;
		case COMPONENT_BODY:
			((struct Body*)data)->prevpos = ((struct Body*)data)->pos;
			iarr_append(&components.bodies, e, data);
			break;
		default:
			ERROR("[ENT] Unhandled component: %u", c);
	}
	entities[e] |= c;
}

void entity_move(Entity e, vec3 dir)
{
	struct Body* body = (struct Body*)iarr_get(components.bodies, e);
	body->forcec = 0;
	body->forces[body->forcec++] = dir;
}

void entities_update()
{
	physics_integrate();
	physics_resolve_collisions();

	/* Update the model matrices */
	mat4* m;
	for (int i = 0; i < components.mats.itemc; i++) {
		m = &((mat4*)components.mats.data)[i];
		mat_set_pos(m, ((struct Body*)components.bodies.data)[i].pos);
	}
}

void entities_free()
{
	DEBUG(1, "[ENT] Freeing entities...");
	iarr_free(&components.mdls  , (void (*)(void*))free_model);
	iarr_free(&components.mats  , NULL);
	iarr_free(&components.lights, NULL);
	iarr_free(&components.bodies, NULL);
}
