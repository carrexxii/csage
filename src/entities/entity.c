#include "util/arraylist.h"
#include "gfx/model.h"
#include "gfx/renderer.h" /* To set the renderer's `renderermdls` pointer */
#include "components.h"
#include "entity.h"

uint64 entityc;
uint64 entities[MAX_ENTITIES];

static struct EntityComponents {
	struct ArrayList models;
} components;

void init_entities()
{
	components.models = create_arrlst(sizeof(struct Model));

	renderermdls = &components.models;

	DEBUG(1, "[ENT] Initialized entities");
}

Entity add_entity()
{
	uint64* e = entities;
	while (*(e++));
	return *e = entityc++ + 1;
}

void add_component(Entity e, enum Component c, union Data data)
{
	struct Model mdl;
	switch (c) {
		case COMPONENT_NONE:
			ERROR("[ENT] Trying to add COMPONENT_NONE");
			break;
		case COMPONENT_MODEL:
			mdl = create_model((char*)data.ptr);
			arrlst_add(components.models, e, &mdl);
			break;
		default:
			ERROR("[ENT] Unhandled component: %u", c);
	}
}
