#include "player.h"

#include "maths/maths.h"
#include "entity.h"

GroupID  player_group;
EntityID player_entity;

void player_init()
{
	player_group  = entity_new_group("player");
	player_entity = entity_new(player_group, NULL);

	DEBUG(1, "[ENT] Initialized player");
}
