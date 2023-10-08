#include "physics.h"
#include "common.h"

#define DAMPNER_VALUE 0.99

void physics_apply_thrust(struct Thruster* thruster, struct Body* body)
{
	CLAMP(thruster->F, 0.0, thruster->F_max);

	body->a[0] += thruster->F*cos(body->θ)/body->m;
	body->a[1] += thruster->F*sin(body->θ)/body->m;

	body->α += thruster->τ/body->I;
}

void physics_integrate(struct Body* body)
{
	body->v[0] *= DAMPNER_VALUE;
	body->v[1] *= DAMPNER_VALUE;
	body->ω *= DAMPNER_VALUE*DAMPNER_VALUE;

	body->v[0] += body->a[0]*dt;
	body->v[1] += body->a[1]*dt;
	body->s[0] += body->v[0]*dt;
	body->s[1] += body->v[1]*dt;

	body->ω += body->α*dt;
	body->θ += body->ω*dt;

	/* Reset */
	body->a[0] = 0.0;
	body->a[1] = 0.0;
	body->α    = 0.0;
}
