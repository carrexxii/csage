#include "maths/maths.h"
#include "util/varray.h"
#include "pathfinding.h"
#include "entity.h"
#include "ai.h"

void ais_update(isize count, struct AI* ais, struct Body* bodies)
{
	struct AI*   ai;
	struct Body* body;
	for (int i = 0; i < count; i++) {
		ai   = &ais[i];
		body = &bodies[i];
		switch (ai->state.type) {
		case AI_STATE_IDLE: break;
		case AI_STATE_FOLLOW:
			assert(ai->state.follow.target);
			if (distance(body->pos, *ai->state.follow.target) > ai->state.follow.dist) {
				body->dir = sub(*ai->state.follow.target, body->pos);
				body->dir_mask = dir_mask_of_vec(body->dir);
			} else {
				body->dir      = VEC2_ZERO;
				body->dir_mask = 0;
			}
			break;
		default:
			ERROR("[ENT] Unmatched AI state: \"%s\"", STRING_OF_AI_STATE_TYPE(ai->state.type));
		}
	}
}
