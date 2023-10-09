#include "missile.h"
#include "gfx/particles.h"
#include "ships/physics.h"
#include "gfx/renderer.h"
#include <cglm/vec2.h>

#define MAX_MISSILES 1024
#define MISSILE_PARTICLE_LIFE     3000
#define MISSILE_PARTICLE_INTERVAL 50

struct Missile missiles[MAX_MISSILES];

ID missile_new(enum MissileType type, vec2 start_pos, vec2 start_vel, ShipID owner)
{
	ID r;
	for (int i = 0; i < MAX_MISSILES; i++)
		if (!missiles[i].owner)
			r = i;

	/* missile types here */
	struct Model model = model_new(MODEL_PATH "missile");
	float particle_scale = 0.1;

	missiles[r] = (struct Missile){
		.type = type,
		.body = (struct Body){
			.s[0] = start_pos[0] + 2.0,
			.s[1] = start_pos[1],
			.v[0] = start_vel[0],
			.v[1] = start_vel[1],
			.m = 3.0,
			.cm = { 0.0, 0.1 },
		},
		.model = model,
		// .particles = particles_new_pool(INT32_MAX, MISSILE_PARTICLE_LIFE, MISSILE_PARTICLE_INTERVAL,
		                                // missiles[r].body.s, start_vel, particle_scale),
		.owner = owner,
	};

	missiles[r].mdli = renderer_add_model(model);

	return r;
}

void missile_set_target(ID missile_id, vec2 target)
{
	if (missile_id >= MAX_MISSILES)
		ERROR("[SHIP] Invalid missile id: %lu", missile_id);
	else if (!missiles[missile_id].owner)
		ERROR("[SHIP] missile %lu does not have an owner", missile_id);
	glm_vec2_copy(target, missiles[missile_id].target);
}

void missiles_update()
{
	mat4* mat;
	struct Missile* missile;
	for (int i = 0; i < MAX_MISSILES; i++) {
		missile = &missiles[i];
		if (!missile->owner)
			continue;

		physics_move_to(&missile->body, missile->target, 1.0, 1.0);
		physics_integrate(&missile->body);

		mat = &renmats[missile->mdli];
		glm_mat4_identity(*mat);
		glm_translate(*mat, (vec3){ missile->body.s[0], missile->body.s[1], 0.0 });
		glm_translate(*mat, (vec3){ missile->body.cm[0], missile->body.cm[1], 0.0 });
		glm_rotate(*mat, missile->body.θ - GLM_PI_2, (vec3){ 0.0, 0.0, 1.0 });
		glm_translate(*mat, (vec3){ -missile->body.cm[0], -missile->body.cm[1], 0.0 });
	}
}

void missiles_free()
{
	struct Missile* r;
	for (int i = 0; i < MAX_MISSILES; i++) {
		r = &missiles[i];
		if (r->owner) {
			model_free(&r->model);
			particles_free_pool(r->particles);
		}
	}
}
