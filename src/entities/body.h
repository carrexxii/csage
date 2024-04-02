#ifndef ENTITIES_BODY_H
#define ENTITIES_BODY_H

#include "maths/types.h"

#define G 0.5f

struct Body {
	Vec2  pos;
	Vec2  facing;
	float vel;
	bool  moving;
};

void bodies_update(int bodyc, struct Body* bodies);

#endif
