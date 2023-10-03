#include "systems.h"

void physics_apply_thrust(struct Thruster thruster)
{
	thruster.parent->a[0] = thruster.F*sinf(thruster.θ - thruster.parent->θ)/thruster.parent->m;
	thruster.parent->a[1] = thruster.F*cosf(thruster.θ - thruster.parent->θ)/thruster.parent->m;

	vec2 F, r;
	float τ;
	F[0] = thruster.F*cosf(thruster.θ);
	F[1] = thruster.F*sinf(thruster.θ);
	glm_vec2_sub(thruster.parent->cm, thruster.s, r);
	τ = glm_vec2_cross(r, F);
	thruster.parent->α = τ/thruster.parent->I;
}

void physics_integrate(struct Body* body)
{
	body->v[0] += body->a[0]*dt;
	body->v[1] += body->a[1]*dt;
	body->s[0] += body->v[0]*dt;
	body->s[1] += body->v[1]*dt;

	body->ω += body->α*dt;
	body->θ += body->ω*dt;
}
