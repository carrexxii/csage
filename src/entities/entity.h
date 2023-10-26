#ifndef ENTITITIES_ENTITY_H
#define ENTITITIES_ENTITY_H

#define MAX_ENTITIES        (UINT16_MAX - 1)
#define STARTING_ARRAY_SIZE 8

typedef uint16 EntityID;

enum EntityComponent {
	COMPONENT_NONE = 0x0,
};

void entity_init();
EntityID entity_new(vec3s pos, char* model_path);
void entity_path_to(EntityID e, ivec3s pos);
void entity_update();
void entity_free();

#endif
