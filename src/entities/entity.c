#include "maths/maths.h"
#include "util/varray.h"
#include "util/arena.h"
#include "body.h"
#include "pathfinding.h"
#include "ai.h"
#include "entity.h"

isize entity_groupc;
struct EntityGroup entity_groups[MAX_ENTITY_GROUPS];

static struct Arena* scratch;

void entities_init()
{
	scratch = arena_new(16*1024*1024, ARENA_RESIZEABLE);
}

void entities_update()
{
	struct EntityGroup* group;
	struct Entity*      entity;
	for (int i = 0; i < entity_groupc; i++) {
		group = &entity_groups[i];
		entities_update_bodies(&group->entities, &group->bodies);
		entities_update_sprites(&group->entities, &group->bodies, group->sheet);
	}
}

void entities_update_bodies(struct VArray* entities, struct VArray* bodies)
{
	struct Body*   body;
	struct Sprite* sprite;
	struct Entity* e;
	for (int i = 0; i < entities->len; i++) {
		e = varray_get(entities, i);
		if (e->id != ENTITY_MAX && e->body != ENTITY_MAX) {
			body = varray_get(bodies, e->body);
			body->vel = multiply(normalized(body->dir), body->speed);
			if (!isnan(body->vel.x) && !isnan(body->vel.y))
				body->pos = add(body->pos, body->vel);
		}
	}
}

void entities_update_sprites(struct VArray* entities, struct VArray* bodies, struct SpriteSheet* sheet)
{
	Vec2* vel;
	struct Entity* entity;
	struct Body*   body;
	struct Sprite* sprite;
	for (int i = 0; i < entities->len; i++) {
		entity = varray_get(entities, i);
		body   = varray_get(bodies, entity->body);
		sprite = varray_get(&sheet->sprites, entity->sprite);
		sprite->pos = body->pos;
	}
}

void entities_free()
{
	for (int i = 0; i < entity_groupc; i++)
		entity_free_group(i);
}

/* -------------------------------------------------------------------- */

GroupID entity_new_group(char* sprite_sheet)
{
	if (entity_groupc >= MAX_ENTITY_GROUPS) {
		ERROR("[ENT] Exceeded the maximum number of entity groups (%d)", MAX_ENTITY_GROUPS);
		return -1;
	}

	entity_groups[entity_groupc] = (struct EntityGroup){
		.count     = 0,
		.sheet     = sprite_sheet_new(sprite_sheet, -2),
		.entities  = varray_new(DEFAULT_ENTITY_COUNT, sizeof(struct Entity)),
		.bodies    = varray_new(DEFAULT_ENTITY_COUNT, sizeof(struct Body)),
	};

	DEBUG(2, "[ENT] Created new entity group using sprite sheet \"%s\"", sprite_sheet);
	return entity_groupc++;
}

void entity_free_group(GroupID gid)
{
	struct EntityGroup* group = &entity_groups[gid];
	// TODO: resmgr handling sprite sheets
	// sprite_sheet_free(group->sheet);
	varray_free(&group->entities);
	varray_free(&group->bodies);
}

/* -------------------------------------------------------------------- */

EntityID entity_new(GroupID gid, struct Body* body)
{
	assert(gid < entity_groupc);

	DEFAULT(body, &default_body);

	struct EntityGroup* group = &entity_groups[gid];
	EntityID eid = varray_push(&group->entities, &(struct Entity){
		.id   = group->count++,
		.body = varray_push(&group->bodies, body),
	});
	sprite_new(group->sheet, 0, body->pos);

	return eid;
}

isize entity_new_batch(GroupID gid, isize entityc, struct Body* bodies)
{
	struct EntityGroup* group = &entity_groups[gid];

	isize fst  = varray_push_many(&group->bodies, entityc, bodies);
	Vec2* poss = arena_alloc(scratch, entityc * sizeof(Vec2));
	for (int i = 0; i < entityc; i++)
		poss[i] = bodies[i].pos;
	sprite_new_batch(group->sheet, 0, entityc, poss, NULL);

	DEBUG(2, "[ENT] Created new entity batch from group %d with %ld entities", gid, entityc);
	return fst;
}

void entity_set_dir(EntityID eid, GroupID gid, enum Direction d, bool set)
{
	struct Body* body = entity_get_body(eid, gid);
	uint dir_mask = body->dir_mask;
	if (set)
		dir_mask |= d;
	else
		dir_mask &= ~d;

	Vec2 dir = VEC2_ZERO;
	if (dir_mask & DIR_N) { dir.x -= 0.5f; dir.y -= 0.5f; }
	if (dir_mask & DIR_S) { dir.x += 0.5f; dir.y += 0.5f; }
	if (dir_mask & DIR_E) { dir.y -= 0.5f; dir.x += 0.5f; }
	if (dir_mask & DIR_W) { dir.y += 0.5f; dir.x -= 0.5f; }

	struct Sprite* sprite = entity_get_sprite(eid, gid);
	if (!dir_mask || dir_mask == (DIR_N | DIR_S)
	              || dir_mask == (DIR_E | DIR_W))
		sprite_set_state(sprite, SPRITE_IDLE, body->dir_mask);
	else if (body->dir_mask != dir_mask)
		sprite_set_state(sprite, SPRITE_RUN, dir_mask);

	body->dir      = dir;
	body->dir_mask = dir_mask;
}
