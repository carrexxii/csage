#include "maths/maths.h"
#include "util/varray.h"
#include "body.h"
#include "pathfinding.h"
#include "ai.h"
#include "entity.h"

struct Entity {
	EntityID pos; // Might not need this one
};

static int groupc;
static struct EntityGroup {
	isize count;
	struct SpriteSheet* sheet;
	struct VArray entities;
	struct VArray positions;
} groups[MAX_ENTITY_GROUPS];

int entity_new_group(char* sprite_sheet)
{
	if (groupc >= MAX_ENTITY_GROUPS) {
		ERROR("[ENT] Exceeded the maximum number of entity groups (%d)", MAX_ENTITY_GROUPS);
		return -1;
	}

	groups[groupc] = (struct EntityGroup){
		.count     = 0,
		.sheet     = sprite_sheet_new(sprite_sheet, -2),
		.entities  = varray_new(DEFAULT_ENTITY_COUNT, sizeof(struct Entity)),
		.positions = varray_new(DEFAULT_ENTITY_COUNT, sizeof(Vec2)),
	};

	DEBUG(2, "[ENT] Created new entity group using sprite sheet \"%s\"", sprite_sheet);
	return groupc++;
}

void entity_free_group(int group_id)
{
	struct EntityGroup* group = &groups[group_id];
	// TODO: resmgr handling sprite sheets
	// sprite_sheet_free(group->sheet);
	varray_free(&group->entities);
	varray_free(&group->positions);
}

/* -------------------------------------------------------------------- */

EntityID entity_new(int group_id, Vec2 pos)
{
	struct EntityGroup* group = &groups[group_id];

	// TODO: freeing/hole-filling
	EntityID id = varray_push(&group->entities, &pos);
	struct Entity* entity = varray_get(&group->entities, id);

	entity->pos = varray_push(&group->positions, &pos);

	sprite_new(group->sheet, 0, VEC3_V2(pos));

	return id;
}

// EntityID entity_new_batch(int group_id, int entityc, Vec2* poss)
// {

// }

/* -------------------------------------------------------------------- */

void entities_update()
{

}

void entities_free()
{
	for (int i = 0; i < groupc; i++)
		entity_free_group(i);
}

/* -------------------------------------------------------------------- */


