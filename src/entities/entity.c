#include "maths/maths.h"
#include "body.h"
#include "pathfinding.h"
#include "ai.h"
#include "entity.h"

isize       entity_groupc;
EntityGroup entity_groups[MAX_ENTITY_GROUPS];

static Arena scratch;

void entities_init()
{
	scratch = arena_new(16*1024*1024, ARENA_RESIZEABLE);
}

void entities_update()
{
	EntityGroup* group;
	Body*        body;
	Sprite*      sprite;
	for (int i = 0; i < entity_groupc; i++) {
		group = &entity_groups[i];
		if (group->ais)
			ais_update(group->count, group->ais, group->bodies);

		bodies_update(group->count, group->bodies);

		for (int j = 0; j < group->count; j++) {
			body = &group->bodies[j];
			sprite = varray_get(&group->sheet->sprites, group->sprites[j]);
			sprite->pos = group->bodies[j].pos;

			if (body->changed_state) {
				SpriteStateType state = body->moving? SPRITE_STATE_RUN: SPRITE_STATE_IDLE;
				sprite_set_state(sprite, state, body->dir_mask);
				body->changed_state = false;
			}
		}
	}
}

void entities_free()
{
	for (int i = 0; i < entity_groupc; i++)
		entity_free_group(i);
}

/* -------------------------------------------------------------------- */

GroupID entity_new_group(char* sprite_sheet, EntityGroupMask mask)
{
	if (entity_groupc >= MAX_ENTITY_GROUPS) {
		ERROR("[ENT] Exceeded the maximum number of entity groups (%d)", MAX_ENTITY_GROUPS);
		return -1;
	}

	EntityGroup* group = &entity_groups[entity_groupc];
	*group = (EntityGroup){
		.count   = 0,
		.cap     = DEFAULT_ENTITY_COUNT,
		.sheet   = sprite_sheet_new(sprite_sheet, -2),
		.bodies  = smalloc(DEFAULT_ENTITY_COUNT * sizeof(Body)),
		.sprites = smalloc(DEFAULT_ENTITY_COUNT * sizeof(EntityID)),
	};
	if (mask & ENTITY_GROUP_AI)
		group->ais = smalloc(DEFAULT_ENTITY_COUNT * sizeof(AI));

	INFO(TERM_ORANGE "[ENT] Created new entity group using sprite sheet \"%s\"", sprite_sheet);
	return entity_groupc++;
}

void entity_resize_group(GroupID gid, isize count)
{
	EntityGroup* group = &entity_groups[gid];
	if (count <= group->cap)
		return;

	group->bodies  = srealloc(group->bodies, count*sizeof(Body));
	group->sprites = srealloc(group->sprites, count*sizeof(EntityID));
	varray_resize(&group->sheet->sprites, count, false);
	if (group->ais)
		group->ais = srealloc(group->ais, count*sizeof(AI));
}

void entity_free_group(GroupID gid)
{
	EntityGroup* group = &entity_groups[gid];
	// TODO: resmgr handling sprite sheets
	// sprite_sheet_free(group->sheet);
	sfree(group->bodies);
	sfree(group->sprites);
	if (group->ais)
		sfree(group->ais);

	group->count = 0;
	group->cap   = 0;
}

/* -------------------------------------------------------------------- */

EntityID entity_new(GroupID gid, EntityCreateInfo* ci)
{
	assert(gid < entity_groupc);

	DEFAULT(ci, (struct EntityCreateInfo*)&default_entity_ci);

	EntityGroup* group = &entity_groups[gid];
	group->bodies[group->count] = (Body){
		.pos   = ci->pos,
		.speed = ci->speed,
	};
	if (group->ais)
		group->ais[group->count] = (AI){
			.type = ci->ai_type,
		};

	group->sprites[group->count] = group->sheet->sprites.len;
	sprite_new(group->sheet, 0, ci->pos);

	return group->count++;
}

isize entity_new_batch(GroupID gid, isize entityc, EntityCreateInfo* cis)
{
	assert(gid < entity_groupc && entityc > 0);

	EntityGroup* group = &entity_groups[gid];
	entity_resize_group(gid, group->count + entityc);

	if (!cis) {
		cis = arena_alloc(&scratch, entityc * sizeof(EntityCreateInfo));
		if (!cis) return -1;
		for (int i = 0; i < entityc; i++)
			cis[i] = default_entity_ci;
	}

	EntityCreateInfo* ci;
	for (int i = group->count; i < group->count + entityc; i++) {
		ci = &cis[i];
		group->bodies[i] = (Body){
			.pos   = ci->pos,
			.speed = ci->speed,
		};

		group->sprites[i] = group->sheet->sprites.len;
		sprite_new(group->sheet, ci->sprite_group, ci->pos);
	}

	if (group->ais)
		for (int i = group->count; i < group->count + entityc; i++)
			group->ais[i] = (AI){
				.type = cis[i].ai_type,
			};

	isize fst = group->count;
	group->count += entityc;
	arena_reset(&scratch);
	INFO(TERM_ORANGE "[ENT] Created new entity batch from group %d with %ld entities", gid, entityc);
	return fst;
}

void entity_set_dir(GroupID gid, EntityID eid, DirectionMask dir, bool set)
{
	Body* body = entity_get_body(gid, eid);
	body_set_dir(body, dir, set);
}

void entity_set_ai_state(GroupID gid, EntityID eid, AIState state)
{
	AI* ai = entity_get_ai(gid, eid);
	ai_set_state(ai, state);
}

