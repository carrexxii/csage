#ifndef ENTITIES_COMPONENTS_H
#define ENTITIES_COMPONENTS_H

enum Component {
	COMPONENT_NONE  = 0x00,
	COMPONENT_MODEL = 0x01,
	COMPONENT_LIGHT = 0x02,
};

struct Light {
	float pos[3];
	float intensity;
}; static_assert(sizeof(struct Light) == 16, "struct Light");

#endif
