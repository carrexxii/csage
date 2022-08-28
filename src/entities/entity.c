#include "util/iarray.h"
#include "gfx/model.h"
#include "components.h"
#include "gfx/renderer.h"
#include "systems.h"
#include "entity.h"

uint64 entityc;
uint64 entities[MAX_ENTITIES];
struct EntityComponents components;

void init_entities()
{
	components.mdls   = create_iarr(sizeof(struct Model), 10);
	components.mats   = create_iarr(sizeof(mat4), 10);
	components.lights = create_iarr(sizeof(vec4), 5);
	components.bodies = create_iarr(sizeof(struct Body), 10);

	renmdlc = &components.mdls.itemc;
	renmdls = components.mdls.data;
	renmats = components.mats.data;

	DEBUG(1, "[ENT] Initialized entities");
}

Entity create_entity()
{
	entities[++entityc] = COMPONENT_NONE;
	return entityc;
}

void add_component(Entity e, enum Component c, void* data)
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
			iarr_append(&components.bodies, e, data);
			break;
		default:
			ERROR("[ENT] Unhandled component: %u", c);
	}
	entities[e] |= c;
}

void update_entities()
{
	apply_forces();
	integrate_bodies();
	resolve_collisions();
}

void free_entities()
{
	DEBUG(1, "[ENT] Freeing entities...");
	free_iarr(&components.mdls  , (void (*)(void*))free_model);
	free_iarr(&components.mats  , NULL);
	free_iarr(&components.lights, NULL);
	free_iarr(&components.bodies, NULL);
}
