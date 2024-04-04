#include "maths/maths.h"
#include "util/varray.h"
#include "pathfinding.h"
#include "entity.h"
#include "ai.h"

void ais_update(isize count, struct AI* ais, struct Body* bodies)
{
	struct AI*      ai;
	struct AIState* state;
	struct Body*    body;
	for (int i = 0; i < count; i++) {
		ai    = &ais[i];
		state = &ai->state;
		body  = &bodies[i];
		switch (ai->state.type) {
		case AI_STATE_IDLE: break;
		case AI_STATE_FOLLOW:
			assert(state->follow.target);
			if (distance(body->pos, *state->follow.target) > state->follow.dist) {
				body->dir      = sub(*state->follow.target, body->pos);
				body->dir_mask = dir_mask_of_vec(body->dir);
			} else {
				body->dir      = VEC2_ZERO;
				body->dir_mask = 0;
			}
			break;
		case AI_STATE_PATROL:
			assert(state->patrol.points);
			Vec2 target = state->patrol.points[state->patrol.i];
			state->patrol.timer += DT_MS;
			if (state->patrol.timer >= state->patrol.delay) {
				if (distance(body->pos, target) <= AI_MOVE_TO_PRECISION) {
					body->dir      = VEC2_ZERO;
					body->dir_mask = 0;
					state->patrol.i     = (state->patrol.i + 1) % state->patrol.pointc;
					state->patrol.timer = 0;
				} else {
					body->dir      = sub(target, body->pos);
					body->dir_mask = dir_mask_of_vec(body->dir);
				}
			}
			break;
		default:
			ERROR("[ENT] Unmatched AI state: \"%s\"", STRING_OF_AI_STATE_TYPE(ai->state.type));
		}
	}
}

