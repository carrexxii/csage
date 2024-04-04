#ifndef ENTITIES_AI_H
#define ENTITIES_AI_H

#include "maths/types.h"
#include "util/varray.h"
#include "components.h"

void ais_update(isize count, struct AI* ais, struct Body* bodies);

/* -------------------------------------------------------------------- */

static inline bool ai_set_state(struct AI* ai, struct AIState state)
{
	bool ret = state.type != ai->state.type;
	ai->state = state;
	
	return ret;
}

#endif

