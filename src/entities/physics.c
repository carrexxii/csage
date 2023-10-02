#include "systems.h"

void physics_apply_thrust(struct Thruster thruster)
{
	thruster.parent->a[0] += thruster.F*sinf(thruster.parent->θ - thruster.θ)/thruster.parent->m;
	thruster.parent->a[1] += thruster.F*cosf(thruster.parent->θ - thruster.θ)/thruster.parent->m;
}

void physics_integrate(struct Body* body)
{
	body->v[0] += body->a[0] * dt;
	body->v[1] += body->a[1] * dt;
	body->s[0] += body->v[0] * dt;
	body->s[1] += body->v[1] * dt;
}
