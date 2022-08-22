#ifndef ENTITIES_COMPONENTS_H
#define ENTITIES_COMPONENTS_H

#define G                    10.0
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
	union Vec3 dim;
	union Vec3 pos;
	union Vec3 vel;
	union Vec3 maxVel;
	union Vec3 acc;
	union Vec3 forces[MAX_FORCES_ON_OBJECT];
	enum Direction dir;
	uint  forcec;
	float mass;
}; static_assert(sizeof(struct Body) == 120, "struct Body");

#endif
