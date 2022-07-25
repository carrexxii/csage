#ifndef ENTITIES_ENTITY_H
#define ENTITIES_ENTITY_H

#include "components.h"

#define MAX_ENTITIES 128

typedef uint64 Entity;

void init_entities();
Entity add_entity();
void add_component(Entity e, enum Component c, union Data data);

#endif
