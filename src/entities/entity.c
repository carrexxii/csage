#include "util/iarray.h"
#include "model.h"
#include "components.h"
#include "entity.h"

uint64 entityc;
uint64 entities[MAX_ENTITIES];
struct EntityComponents components;

void init_entities()
{
	components.mdls   = create_iarr(sizeof(struct Model), 0);
	components.lights = create_iarr(sizeof(struct Light), 0);

	DEBUG(1, "[ENT] Initialized entities");
}

Entity add_entity()
{
	uint64* e = entities;
	while (*(e++));
	return *e = entityc++ + 1;
}

void add_component(Entity e, enum Component c, void* data)
{
	switch (c) {
		case COMPONENT_NONE:
			ERROR("[ENT] Trying to add COMPONENT_NONE");
			break;
		case COMPONENT_MODEL: iarr_append(&components.mdls  , e, (struct Model*)data); break;
		case COMPONENT_LIGHT: iarr_append(&components.lights, e, (struct Light*)data); break;
		default:
			ERROR("[ENT] Unhandled component: %u", c);
	}
}
