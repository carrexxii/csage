#include "util/iarray.h"
#include "components.h"
#include "gfx/renderer.h"
#include "camera.h"
#include "systems.h"
#include "entity.h"

intptr entityc;
Entity entities[MAX_ENTITIES];
struct EntityComponents components;

void entities_init()
{
	components.mats = iarr_new(sizeof(mat4), 10);

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
	// struct Model mdl;
	switch (c) {
		case COMPONENT_NONE:
			ERROR("[ENT] Trying to add COMPONENT_NONE");
			break;
		// case COMPONENT_MODEL:
		// 	mdl = create_model((char*)data);
		// 	iarr_append(&components.mdls, e, &mdl);
		// 	iarr_append(&components.mats, e, GLM_MAT4_IDENTITY);
		// 	break;
		default:
			ERROR("[ENT] Unhandled component add: %u", c);
	}
	entities[e] |= c;
}

void entities_update()
{

}

void entities_free()
{
	DEBUG(1, "[ENT] Freeing entities...");
	iarr_free(&components.mats, NULL);
}
