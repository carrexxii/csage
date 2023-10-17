#ifndef ENTITIES_BODY_H
#define ENTITIES_BODY_H

#include "component.h"

struct Body {
	vec3s pos;
	vec3s facing;
	float vel;
	bool moving;
};

void bodies_update(struct ComponentArray* bodies);

#endif
