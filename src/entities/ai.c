#include "maths/maths.h"
#include "pathfinding.h"
#include "components.h"
#include "body.h"
#include "ai.h"

void ais_update(isize count, AI* ais, Body* bodies)
{
	AI*      ai;
	AIState* state;
	Body*    body;
	for (int i = 0; i < count; i++) {
		ai    = &ais[i];
		state = &ai->state;
		body  = &bodies[i];
		switch (ai->state.type) {
		case AI_STATE_IDLE: break;
		case AI_STATE_FOLLOW:
			assert(state->follow.target);

			if (distance(body->pos, *state->follow.target) > state->follow.dist) {
				body->dir_mask = 0;
				Vec2 dir = sub(*state->follow.target, body->pos);
				body_set_dir(body, mask_of_dir(dir), true);
			} else if (body->moving) {
				body_set_dir(body, DIR_ALL, false);
			}
			break;
		case AI_STATE_PATROL:
			assert(state->patrol.points);

			Vec2 target = state->patrol.points[state->patrol.i];
			state->patrol.timer += DT_MS;
			if (state->patrol.timer >= state->patrol.delay) {
				if (distance(body->pos, target) > AI_MOVE_TO_PRECISION) {
					body->dir_mask = 0;
					Vec2 dir = sub(target, body->pos);
					body_set_dir(body, mask_of_dir(dir), true);
				} else {
					state->patrol.i     = (state->patrol.i + 1) % state->patrol.pointc;
					state->patrol.timer = 0;
				}
			} else {
				if (body->moving)
					body_set_dir(body, DIR_ALL, false);
			}
			break;
		default:
			ERROR("[ENT] Unmatched AI state: \"%s\"", STRING_OF_AI_STATE_TYPE(ai->state.type));
		}
	}
}

