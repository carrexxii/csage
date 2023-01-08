#ifndef ENTITIES_ENTITY_H
#define ENTITIES_ENTITY_H

#include "util/iarray.h"
#include "components.h"

#define MAX_ENTITIES 128

typedef uint64 Entity;

void   entities_init();
Entity entity_new();
void   entity_add_component(Entity e, enum Component c, void* data);
void   entity_move(Entity e, vec3 dir);
bool   entity_select_by_ray(struct Ray r);
void   entity_set_body_dimensions(Entity e);
void   entities_update();
void   entities_free();

extern struct EntityComponents {
	struct IArray mdls;
	struct IArray mats;
	struct IArray lights;
	struct IArray bodies;
	struct IArray actors;
} components;

extern intptr entityc;
extern Entity entities[];

#endif

