#ifndef ENTITITIES_ENTITY_H
#define ENTITITIES_ENTITY_H

#include "maths/types.h"
#include "gfx/sprite.h"

#define MAX_ENTITY_GROUPS      8
#define MAX_ENTITIES_PER_GROUP (UINT16_MAX - 1)
#define DEFAULT_ENTITY_COUNT   8
#define ENTITY_PATH_EPSILON    0.1 /* Will be compared to the square of the distance */

typedef uint EntityID;

int  entity_new_group(char* sprite_sheet);
void entity_free_group(int group_id);

EntityID entity_new(int group, Vec2 pos);
void     entity_path_to(EntityID e, Vec3i pos);

void entities_free(void);
void entities_update(void);

#endif
