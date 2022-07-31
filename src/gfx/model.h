#ifndef GFX_MODEL_H
#define GFX_MODEL_H

#include "cglm/cglm.h"

#include "gfx/buffers.h"

struct Material {
	float rgb[3];
}; static_assert(sizeof(struct Material) == 12, "struct Material");

struct Model {
	VBO    vbo;
	float* verts;
	struct Material* materials; /* TODO: These don't need to be stored after mesh is loaded */
	uint16 vertc;
	uint8  materialc;
}; static_assert(sizeof(struct Model) == 48, "struct Model");

struct Model create_model(char* const path);

#endif
