#ifndef ENTITIES_SYSTEMS_H
#define ENTITIES_SYSTEMS_H

#include "entity.h"

void physics_integrate();
void physics_resolve_collisions();

bool collisions_map(struct Body* body);

void actors_update();
void actor_free(struct Actor* actor);

#endif

