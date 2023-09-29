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
	components.mats = iarr_new(sizeof(mat4), 10);
	components.mdls = iarr_new(sizeof(struct Model), 10);

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
	switch (c) {
		case COMPONENT_NONE:
			ERROR("[ENT] Trying to add COMPONENT_NONE");
			break;
		case COMPONENT_MODEL:
			struct Model mdl = *(struct Model*)data;
			iarr_append(&components.mdls, e, &mdl);
			break;
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
	iarr_free(&components.mdls, (void (*)(void*))model_free);
}
