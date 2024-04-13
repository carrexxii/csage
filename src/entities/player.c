#include "player.h"

#include "maths/maths.h"
#include "entity.h"

GroupID  player_group;
EntityID player_entity;

void player_init()
{
	player_group  = entity_new_group("player", 0);
	player_entity = entity_new(player_group, &(struct EntityCreateInfo){
		.speed = 0.2f,
	});

	INFO(TERM_ORANGE "[ENT] Initialized player");
}

