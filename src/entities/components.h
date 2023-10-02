#ifndef ENTITIES_COMPONENTS_H
#define ENTITIES_COMPONENTS_H

#include "cglm/cglm.h"

enum Component {
	COMPONENT_NONE     = 0x00,
	COMPONENT_MODEL    = 0x01,
	COMPONENT_BODY     = 0x02,
	COMPONENT_THRUSTER = 0x04,
};

struct Body {
	float θ;
	float m, I;
	vec2 s, v, a;
	vec2 ω, α;
};

struct Thruster {
	struct Body* parent;
	float θ;
	vec2 s;
	float F, Fmin, Fmax;
};

#endif
