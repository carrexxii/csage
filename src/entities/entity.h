#ifndef ENTITIES_ENTITY_H
#define ENTITIES_ENTITY_H

#include "util/iarray.h"
#include "components.h"

#define MAX_ENTITIES 128

typedef uint64 Entity;

void   init_entities();
Entity create_entity();
void   add_component(Entity e, enum Component c, void* data);
void   free_entities();

extern struct EntityComponents {
	struct IArray mdls;
	struct IArray mats;
	struct IArray lights;
} components;

extern uint64 entityc;
extern uint64 entities[];

#endif
