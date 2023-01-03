#ifndef ENTITIES_COMPONENTS_H
#define ENTITIES_COMPONENTS_H

#define G 1.0

#define BODY_MAX_FORCES 4

enum Component {
	COMPONENT_NONE  = 0x00,
	COMPONENT_MODEL = 0x01,
	COMPONENT_LIGHT = 0x02,
	COMPONENT_BODY  = 0x04,
};

enum Shape {
	SHAPE_NONE,
	SHAPE_CUBOID,
	SHAPE_CYLINDER,
};

struct Body {
	vec3 pos;
	vec3 prevpos;
	vec3 dim;
	vec3 forces[BODY_MAX_FORCES];
	uint8 forcec;
}; static_assert(sizeof(struct Body) == 88, "struct Body");

#endif
