#ifndef ENTITIES_SYSTEMS_H
#define ENTITIES_SYSTEMS_H

#include "entity.h"

// TODO: inline these?
void physics_apply_thrust(struct Thruster thruster);
void physics_integrate(struct Body* body);

#endif

