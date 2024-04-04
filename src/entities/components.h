#ifndef ENTITIES_COMPONENTS_H
#define ENTITIES_COMPONENTS_H

#include "maths/types.h"

struct Body {
	Vec2  pos;
	float speed;
	Vec2  vel;
	enum Direction dir_mask;
	bool moving;
	bool changed_state;
};

/* -------------------------------------------------------------------- */

#define AI_MOVE_TO_PRECISION 0.5f

#define STRING_OF_AI_TYPE(x) ((x) >= AI_TYPE_MAX? "<Unknown>": string_of_ai_type[x])
enum AIType {
	AI_TYPE_NONE,
	AI_TYPE_CONTROLLABLE,
	AI_TYPE_FRIENDLY,
	AI_TYPE_NEUTRAL,
	AI_TYPE_ENEMY,
	AI_TYPE_MAX,
};
static const char* string_of_ai_type[] = {
	[AI_TYPE_NONE]         = "AI_TYPE_NONE",
	[AI_TYPE_CONTROLLABLE] = "AI_TYPE_CONTROLLABLE",
	[AI_TYPE_FRIENDLY]     = "AI_TYPE_FRIENDLY",
	[AI_TYPE_NEUTRAL]      = "AI_TYPE_NEUTRAL",
	[AI_TYPE_ENEMY]        = "AI_TYPE_ENEMY",
};

#define STRING_OF_AI_STATE_TYPE(x) ((x) >= AI_STATE_MAX? "<Unknown>": string_of_ai_state_type[x])
enum AIStateType {
	AI_STATE_IDLE,
	AI_STATE_FOLLOW,
	AI_STATE_PATROL,
	AI_STATE_WANDER,
	AI_STATE_PATHING,
	AI_STATE_MAX,
};
static const char* string_of_ai_state_type[] = {
	[AI_STATE_IDLE]    = "AI_STATE_IDLE",
	[AI_STATE_FOLLOW]  = "AI_STATE_FOLLOW",
	[AI_STATE_PATROL]  = "AI_STATE_PATROL",
	[AI_STATE_WANDER]  = "AI_STATE_WANDER",
	[AI_STATE_PATHING] = "AI_STATE_PATHING",
};
struct AIState {
	enum AIStateType type;
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
};

struct AI {
	enum AIType    type;
	struct AIState state;
};
static const struct AI default_ai = {
	.type  = AI_TYPE_NEUTRAL,
	.state = AI_STATE_IDLE,
};

/* -------------------------------------------------------------------- */

typedef int GroupID;
typedef int EntityID;

enum EntityGroupMask {
	ENTITY_GROUP_NONE = 0,
	ENTITY_GROUP_AI   = 1 << 0,
};

struct EntityGroup {
	isize count;
	isize cap;
	struct SpriteSheet* sheet;
	isize sheet_group;
	struct Body* bodies;
	EntityID*    sprites;
	struct AI*   ais;
};

struct EntityCreateInfo {
	Vec2  pos;
	float speed;
	int sprite_group;
	enum AIType ai_type;
};

/* -------------------------------------------------------------------- */

#endif

