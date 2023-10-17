#ifndef ENTITITIES_ENTITY_H
#define ENTITITIES_ENTITY_H

typedef uint16 Entity;

enum EntityComponent {
	COMPONENT_NONE = 0x0,
};

void   entity_init();
Entity entity_new(vec3s pos, char* model_path);
void   entity_update();
void   entity_free();

#endif
