#ifndef ENTITIES_BODY_H
#define ENTITIES_BODY_H

#include "maths/maths.h"

#define G 0.5f

struct Body {
	Vec3  pos;
	Vec3  facing;
	float vel;
	bool  moving;
};

void bodies_update(int bodyc, struct Body* bodies);

#endif
