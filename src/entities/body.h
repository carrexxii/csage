#ifndef ENTITIES_BODY_H
#define ENTITIES_BODY_H

#include "maths/types.h"

#define G 0.5f

struct Body {
	Vec2  pos;
	float speed;
	Vec2  vel;
	uint  dir_mask;
	Vec2  dir;
};

static const struct Body default_body = {
	.speed = 0.1f,
};

void bodies_update(int bodyc, struct Body* bodies);

#endif
