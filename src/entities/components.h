#ifndef ENTITIES_COMPONENTS_H
#define ENTITIES_COMPONENTS_H

#include "common.h"
#include "maths/types.h"
#include "gfx/sprite.h"

typedef struct Body {
	Vec2  pos;
	float speed;
	Vec2  vel;
	DirectionMask dir_mask;
	bool moving;
	bool changed_state;
} Body;

/* -------------------------------------------------------------------- */

#define AI_MOVE_TO_PRECISION 0.5f

typedef enum AIType {
	AI_TYPE_NONE,
	AI_TYPE_CONTROLLABLE,
	AI_TYPE_FRIENDLY,
	AI_TYPE_NEUTRAL,
	AI_TYPE_ENEMY,
	AI_TYPE_MAX,
} AIType;

typedef enum AIStateType {
	AI_STATE_IDLE,
	AI_STATE_FOLLOW,
	AI_STATE_PATROL,
	AI_STATE_WANDER,
	AI_STATE_PATHING,
	AI_STATE_MAX,
} AIStateType;

typedef struct AIState {
	AIStateType type;
	union {
		struct {
			Vec2* target;
			float dist;
		} follow;
		struct {
			uint8  pointc, i;
			uint16 delay, timer;
			Vec2*  points;
		} patrol;
	};
} AIState;

typedef struct AI {
	AIType  type;
	AIState state;
} AI;
static const AI default_ai = {
	.type  = AI_TYPE_NEUTRAL,
	.state = AI_STATE_IDLE,
};

/* -------------------------------------------------------------------- */

typedef int GroupID;
typedef int EntityID;

typedef enum EntityGroupMask {
	ENTITY_GROUP_NONE = 0,
	ENTITY_GROUP_AI   = 1 << 0,
} EntityGroupMask;

typedef struct EntityGroup {
	SpriteSheet* sheet;
	isize        count;
	isize        cap;
	isize        sheet_group;
	Body*        bodies;
	EntityID*    sprites;
	struct AI*   ais;
} EntityGroup;

typedef struct EntityCreateInfo {
	Vec2   pos;
	float  speed;
	int    sprite_group;
	AIType ai_type;
} EntityCreateInfo;

#endif

