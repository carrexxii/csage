#ifndef ENTITIES_COMPONENTS_H
#define ENTITIES_COMPONENTS_H

#define G 10.0

#define BODY_MAX_FORCES 4
#define CAPSULE_RADIUS_PERCENTAGE 0.2

enum Component {
	COMPONENT_NONE         = 0x00,
	COMPONENT_MODEL        = 0x01,
	COMPONENT_LIGHT        = 0x02,
	COMPONENT_BODY         = 0x04,
	COMPONENT_ACTOR        = 0x08,
	COMPONENT_CONTROLLABLE = 0x10,
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
	float dir;
	uint8 forcec;
	vec3  forces[BODY_MAX_FORCES];
}; static_assert(sizeof(struct Body) == 92, "struct Body");

enum ActorState {
	ACTOR_NONE = 0,
	ACTOR_IDLING,
	ACTOR_PATHING,
};

struct Actor {
	enum ActorState state;
	ivec3  dest;
	ivec3* path;
}; static_assert(sizeof(struct Actor) == 24, "struct Actor");

#endif

