#ifndef ENTITIES_AI_H
#define ENTITIES_AI_H

#include "common.h"
#include "maths/types.h"
#include "components.h"

void ais_update(isize count, AI* ais, Body* bodies);

/* -------------------------------------------------------------------- */

static inline bool ai_set_state(AI* ai, AIState state)
{
	bool ret = state.type != ai->state.type;
	ai->state = state;

	return ret;
}

#endif

