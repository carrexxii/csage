#ifndef ENTITITIES_ENTITY_H
#define ENTITITIES_ENTITY_H

#include "maths/types.h"
#include "gfx/sprite.h"

#define MAX_ENTITIES        (UINT16_MAX - 1)
#define STARTING_ARRAY_SIZE 8
#define ENTITY_PATH_EPSILON 0.1 /* Will be compared to the square of the distance */

typedef uint16 Entity;

enum EntityComponent {
	COMPONENT_NONE = 0x0,
};

void   entities_init(void);
Entity entity_new(struct SpriteSheet* sheet, Vec2 pos);
void   entity_path_to(Entity e, Vec3i pos);
void   entities_update(void);
void   entities_free(void);

#endif
