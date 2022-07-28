#ifndef ENTITIES_ENTITY_H
#define ENTITIES_ENTITY_H

#include "util/iarray.h"
#include "components.h"

#define MAX_ENTITIES 128

typedef uint64 Entity;

extern struct EntityComponents {
	struct IArray mdls;
	struct IArray lights;
} components;

void init_entities();
Entity add_entity();
void add_component(Entity e, enum Component c, void* data);

#endif
