#ifndef ENTITIES_SYSTEMS_H
#define ENTITIES_SYSTEMS_H

#include "entity.h"

void physics_integrate();
void physics_resolve_collisions();

bool collision_map(struct Body* body);
bool collision_ray(struct Body* body, struct Ray ray);

void actors_update();
void actor_free(struct Actor* actor);

#endif

