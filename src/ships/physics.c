#include "physics.h"
#include "common.h"

void physics_apply_thrust(struct Thruster* thruster, struct Body* body)
{
	// CLAMP(thruster->F, thruster->Fmin, thruster->Fmax); // TODO: do this elsewhere?
	float Δθ = body->θ - thruster->θ;

	// !!! TODO: take into account ship's position/rotation
	
	/* Translation */
	body->a[0] += thruster->F*sinf(Δθ)/body->m;
	body->a[1] += thruster->F*cosf(Δθ)/body->m;

	/* Rotation */
	vec2 F, r;
	float τ;
	F[0] = thruster->F*sinf(thruster->θ);
	F[1] = thruster->F*cosf(thruster->θ);
	glm_vec2_sub(thruster->s, body->cm, r);
	τ = glm_vec2_cross(r, F);
	body->α += τ/body->I;
}

void physics_integrate(struct Body* body)
{
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
