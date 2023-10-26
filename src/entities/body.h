#ifndef ENTITIES_BODY_H
#define ENTITIES_BODY_H

#define G 0.5f

struct Body {
	vec3s pos;
	vec3s facing;
	float vel;
	bool moving;
};

void bodies_update(int bodyc, struct Body* bodies);

#endif
