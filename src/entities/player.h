#ifndef ENTITIES_PLAYER_H
#define ENTITIES_PLAYER_H

#include "common.h"
#include "maths/types.h"
#include "entity.h"

extern GroupID  player_group;
extern EntityID player_entity;

void player_init(void);

static inline void player_set_moving(DirectionMask dir, bool set)
{
	entity_set_dir(player_entity, player_group, dir, set);
}

#endif

