#ifndef ENTITIES_COMPONENTS_H
#define ENTITIES_COMPONENTS_H

#define G                    UPS(1.0)
#define MAX_VEL              UPS(1.5)
#define MAX_ACC              UPS(0.1)
#define DAMPNER              0.01
#define MAX_FORCES_ON_OBJECT 4

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
	vec3 dim;
	vec3 pos;
	vec3 vel;
	vec3 maxVel;
	vec3 acc;
	vec3 forces[MAX_FORCES_ON_OBJECT];
	uint  forcec;
	float mass;
}; static_assert(sizeof(struct Body) == 116, "struct Body");

#endif
