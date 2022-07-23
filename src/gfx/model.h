#ifndef GFX_MODEL_H
#define GFX_MODEL_H

#include "buffers.h"

struct Material {
	float rgb[3];
}; //static_assert(sizeof(struct Material) == , "struct Material");

struct Model {
	uint16 vertc;
	uint8  materialc;
	float* verts;
	struct Material* materials;
	VBO vbo;
}; //static_assert(sizeof(struct Model) == 8, "struct Model");

struct Model create_model(char* const path);

#endif
