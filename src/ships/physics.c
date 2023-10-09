#include "physics.h"
#include "common.h"
#include <cglm/io.h>
#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <math.h>

#define DAMPNER_VALUE 0.99

void physics_apply_force(struct Body* body, vec2 F)
{
	body->a[0] += F[0]/body->m;
	body->a[1] += F[1]/body->m;
}

void physics_apply_torque(struct Body* body, float τ)
{
	if (body->I)
		body->α += τ/body->I;
	else
		body->α += τ/body->m;
}

void physics_apply_thrust(struct Thruster* thruster, struct Body* body)
{
	CLAMP(thruster->F, 0.0, thruster->F_max);
	physics_apply_force(body, (vec2){ thruster->F*cos(body->θ), thruster->F*sin(body->θ) });
	physics_apply_torque(body, thruster->τ);
}

void physics_move_to(struct Body* body, vec2 target, float F_max, float τ_max)
{
	vec2 facing = { cosf(body->θ), sinf(body->θ) };

	vec2 F;
	glm_vec2_scale(facing, F_max, F);
	physics_apply_force(body, F);

	vec2 diff;
	glm_vec2_sub(body->s, target, diff);
	physics_apply_torque(body, glm_vec2_cross(diff, facing) > 0? τ_max: -τ_max);
}

void physics_integrate(struct Body* body)
{
	if (!body->no_dampen) {
		body->v[0] *= DAMPNER_VALUE;
		body->v[1] *= DAMPNER_VALUE;
		body->ω *= DAMPNER_VALUE*DAMPNER_VALUE;
	}

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
