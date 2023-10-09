#ifndef PHYSICS_H
#define PHYSICS_H

#include "gfx/particles.h"

struct Body {
	float* verts;
	int    vertc;
	float  θ;
	float  m, I;
	vec2   cm;
	vec2   s, v, a;
	float  ω, α;
	bool   no_dampen;
};

struct Thruster {
	vec2  s;
	float F, F_max;
	float τ, τ_max;
	int   particles;
};

void physics_apply_thrust(struct Thruster* thruster, struct Body* body);
void physics_move_to(struct Body* body, vec2 target, float F_max, float τ_max);
void physics_integrate(struct Body* body);

#endif
