#include "entity.h"
#include "components.h"
#include "systems.h"

void actors_update()
{
	struct Actor* actor;
	for (int i = 0; i < components.actors.itemc; i++) {
		actor = &((struct Actor*)components.actors.data)[i];
		if (!actor->state)
			actor->state = ACTOR_IDLING;
	}
}

void actor_free(struct Actor* actor)
{
	if (actor->path)
		free(actor->path);
}

