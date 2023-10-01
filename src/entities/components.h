#ifndef ENTITIES_COMPONENTS_H
#define ENTITIES_COMPONENTS_H

#include "cglm/cglm.h"

enum Component {
	COMPONENT_NONE  = 0x00,
	COMPONENT_MODEL = 0x01,
	COMPONENT_BODY  = 0x02,
};

struct Body {
	vec2 pos;
};

#endif

