#ifndef ENTITIES_COMPONENTS_H
#define ENTITIES_COMPONENTS_H

#define G 10.0

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
}; static_assert(sizeof(struct Body) == 36, "struct Body");

#endif
