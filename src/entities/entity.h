#ifndef ENTITITIES_ENTITY_H
#define ENTITITIES_ENTITY_H

#include "maths/types.h"
#include "gfx/sprite.h"
#include "body.h"

#define ENTITY_MAX             UINT_MAX
#define MAX_ENTITY_GROUPS      8
#define MAX_ENTITIES_PER_GROUP (UINT16_MAX - 1)
#define DEFAULT_ENTITY_COUNT   8
#define ENTITY_PATH_EPSILON    0.1 /* Will be compared to the square of the distance */

enum ComponentType {
	COMPONENT_NONE,
	COMPONENT_POS,
	COMPONENT_SPRITE,
};
static const char* string_of_component_type[] = {
	[COMPONENT_NONE]   = "COMPONENT_NONE",
	[COMPONENT_POS]    = "COMPONENT_POS",
	[COMPONENT_SPRITE] = "COMPONENT_SPRITE",
};

typedef uint GroupID;
typedef uint EntityID;

struct Entity {
	EntityID id;
	EntityID body;
	EntityID sprite;
};

extern isize entity_groupc;
extern struct EntityGroup {
	isize count;
	struct SpriteSheet* sheet;
	struct VArray entities;
	struct VArray bodies;
} entity_groups[MAX_ENTITY_GROUPS];

GroupID entity_new_group(char* sprite_sheet);
void    entity_free_group(GroupID gid);

EntityID entity_new(GroupID gid, struct Body* body);
isize    entity_new_batch(GroupID gid, isize entityc, struct Body* bodies);
// void     entity_path_to(EntityID e, Vec3i pos);
void entity_set_dir(EntityID eid, GroupID gid, enum Direction dir, bool set);

void entities_init(void);
void entities_free(void);
void entities_update(void);
void entities_update_bodies(struct VArray* entities, struct VArray* bodies);
void entities_update_sprites(struct VArray* entities, struct VArray* bodies, struct SpriteSheet* sheet);

/* -------------------------------------------------------------------- */

#define EGET(type, name, arr)                                               \
	[[gnu::always_inline]]                                                   \
	static inline type entity_get_##name(EntityID eid, GroupID gid) {         \
		struct Entity* entity = varray_get(&entity_groups[gid].entities, eid); \
		return varray_get(&entity_groups[gid].arr, entity->name);               \
	}
EGET(struct Body*  , body  , bodies        )
EGET(struct Sprite*, sprite, sheet->sprites)
#undef EGET

#endif
