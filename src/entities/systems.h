#ifndef ENTITIES_SYSTEMS_H
#define ENTITIES_SYSTEMS_H

#include "maths/maths.h"

#include "entity.h"

void entity_set_pos(Entity e, vec3 pos);
void physics_integrate();

#endif

