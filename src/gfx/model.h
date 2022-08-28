#ifndef GFX_MODEL_H
#define GFX_MODEL_H

#include "gfx/buffers.h"

struct Material {
	vec3 rgb;
}; static_assert(sizeof(struct Material) == 12, "struct Material");

struct Model {
	VBO    vbo;
	float* verts; /* TODO: Remove this? */
	struct Material* materials; /* TODO: These don't need to be stored after mesh is loaded */
	uint16 vertc;
	uint8  materialc;
}; static_assert(sizeof(struct Model) == 48, "struct Model");

struct Model create_model(char* const path);
void free_model(struct Model* mdl);

#endif
