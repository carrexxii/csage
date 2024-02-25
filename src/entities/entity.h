#ifndef ENTITITIES_ENTITY_H
#define ENTITITIES_ENTITY_H

#include "maths/maths.h"

#define MAX_ENTITIES        (UINT16_MAX - 1)
#define STARTING_ARRAY_SIZE 8
#define ENTITY_PATH_EPSILON 0.1 /* Will be compared to the square of the distance */

typedef uint16 EntityID;

enum EntityComponent {
	COMPONENT_NONE = 0x0,
};

void entities_init();
EntityID entity_new(Vec3 pos, char* model_path);
void entity_path_to(EntityID e, Vec3i pos);
void entities_update();
void entities_free();

#endif
