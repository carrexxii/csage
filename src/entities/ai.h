#ifndef ENTITIES_AI_H
#define ENTITIES_AI_H

#include "maths/types.h"
#include "util/varray.h"
#include "body.h"

#define AI_MOVE_TO_PRECISION 0.5f

#define STRING_OF_AI_TYPE(x) ((x) >= AI_TYPE_MAX? "<Unknown>": string_of_ai_type[x])
enum AIType {
	AI_TYPE_NONE,
	AI_TYPE_CONTROLLABLE,
	AI_TYPE_FRIENDLY,
	AI_TYPE_NEUTRAL,
	AI_TYPE_ENEMY,
	AI_TYPE_MAX,
};
static const char* string_of_ai_type[] = {
	[AI_TYPE_NONE]         = "AI_TYPE_NONE",
	[AI_TYPE_CONTROLLABLE] = "AI_TYPE_CONTROLLABLE",
	[AI_TYPE_FRIENDLY]     = "AI_TYPE_FRIENDLY",
	[AI_TYPE_NEUTRAL]      = "AI_TYPE_NEUTRAL",
	[AI_TYPE_ENEMY]        = "AI_TYPE_ENEMY",
};

#define STRING_OF_AI_STATE_TYPE(x) ((x) >= AI_STATE_MAX? "<Unknown>": string_of_ai_state_type[x])
enum AIStateType {
	AI_STATE_IDLE,
	AI_STATE_FOLLOW,
	AI_STATE_PATROL,
	AI_STATE_WANDER,
	AI_STATE_PATHING,
	AI_STATE_MAX,
};
static const char* string_of_ai_state_type[] = {
	[AI_STATE_IDLE]    = "AI_STATE_IDLE",
	[AI_STATE_FOLLOW]  = "AI_STATE_FOLLOW",
	[AI_STATE_PATROL]  = "AI_STATE_PATROL",
	[AI_STATE_WANDER]  = "AI_STATE_WANDER",
	[AI_STATE_PATHING] = "AI_STATE_PATHING",
};
struct AIState {
	enum AIStateType type;
	union {
		struct {
			Vec2* target;
			float dist;
		} follow;
		struct {
			uint8  pointc, i;
			uint16 delay, timer;
			Vec2*  points;
		} patrol;
	};
};

struct AI {
	enum AIType    type;
	struct AIState state;
};
static const struct AI default_ai = {
	.type  = AI_TYPE_NEUTRAL,
	.state = AI_STATE_IDLE,
};

void ais_update(isize count, struct AI* ais, struct Body* bodies);

/* -------------------------------------------------------------------- */

static inline void ai_set_state(struct AI* ai, struct AIState state)
{
	ai->state = state;
}

#endif

