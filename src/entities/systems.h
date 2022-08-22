#ifndef ENTITIES_SYSTEMS_H
#define ENTITIES_SYSTEMS_H

#include "cglm/cglm.h"

#include "entity.h"

void set_entity_pos(Entity e, vec3 pos);
void apply_forces();
void integrate_bodies();
void resolve_collisions();

#endif
