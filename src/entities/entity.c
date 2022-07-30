#include "util/iarray.h"
#include "model.h"
#include "components.h"
#include "entity.h"
#include <cglm/mat4.h>

uint64 entityc;
uint64 entities[MAX_ENTITIES];
struct EntityComponents components;

void init_entities()
{
	components.mdls   = create_iarr(sizeof(struct Model), 0);
	components.mats   = create_iarr(sizeof(mat4), 0);
	components.lights = create_iarr(sizeof(struct Light), 0);

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
	switch (c) {
		case COMPONENT_NONE:
			ERROR("[ENT] Trying to add COMPONENT_NONE");
			break;
		case COMPONENT_MODEL:
			mdl = create_model((char*)data);
			iarr_append(&components.mdls, e, &mdl);
			iarr_append(&components.mats, e, GLM_MAT4_IDENTITY);
			break;
		case COMPONENT_LIGHT:
			iarr_append(&components.lights, e, (struct Light*)data);
			break;
		default:
			ERROR("[ENT] Unhandled component: %u", c);
	}
	entities[e] |= c;
}
