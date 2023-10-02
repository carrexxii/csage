#ifndef ENTITIES_SYSTEMS_H
#define ENTITIES_SYSTEMS_H

#include "entity.h"
#include "gfx/polygon.h"

struct Body body_new(int polyc, struct Polygon* polys, vec2 s, float m);

// TODO: inline these?
void physics_apply_thrust(struct Thruster thruster);
void physics_integrate(struct Body* body);

#endif
