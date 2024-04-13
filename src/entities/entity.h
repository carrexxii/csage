#ifndef ENTITITIES_ENTITY_H
#define ENTITITIES_ENTITY_H

#include "common.h"
#include "maths/types.h"
#include "gfx/sprite.h"
#include "components.h"
#include "body.h"
#include "ai.h"

#define ENTITY_MAX             INT_MAX
#define MAX_ENTITY_GROUPS      8
#define MAX_ENTITIES_PER_GROUP (UINT16_MAX - 1)
#define DEFAULT_ENTITY_COUNT   8
#define ENTITY_PATH_EPSILON    0.1 /* Will be compared to the square of the distance */

extern isize       entity_groupc;
extern EntityGroup entity_groups[MAX_ENTITY_GROUPS];

static const EntityCreateInfo default_entity_ci = {
	.speed   = 0.1f,
	.ai_type = AI_TYPE_NEUTRAL,
};

void entities_init(void);
void entities_free(void);
void entities_update(void);

GroupID entity_new_group(char* sprite_sheet, EntityGroupMask mask);
void    entity_resize_group(GroupID gid, isize count);
void    entity_free_group(GroupID gid);

EntityID entity_new(GroupID gid, EntityCreateInfo* ci);
isize    entity_new_batch(GroupID gid, isize entityc, EntityCreateInfo* cis);
void entity_set_dir(GroupID gid, EntityID eid, DirectionMask dir, bool set);
void entity_set_ai_state(GroupID gid, EntityID eid, AIState state);

/* -------------------------------------------------------------------- */

#define EGET(type, name, arr)                                        \
	[[gnu::always_inline]]                                            \
	static inline type entity_get_##name(GroupID gid, EntityID eid) {  \
		assert(gid < entity_groupc && eid < entity_groups[gid].count);  \
		return &entity_groups[gid].arr[eid];                             \
	}
EGET(Body*, body, bodies)
EGET(AI*  , ai  , ais   )
#undef EGET

[[gnu::always_inline]]
static inline Sprite* entity_get_sprite(GroupID gid, EntityID eid)
{
	assert(gid < entity_groupc && eid < entity_groups[gid].count);

	EntityGroup* group   = &entity_groups[gid];
	EntityID*    sprites = group->sprites;
	return varray_get(&group->sheet->sprites, sprites[eid]);
}

#endif

