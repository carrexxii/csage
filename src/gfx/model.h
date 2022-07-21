#ifndef GFX_MODEL_H
#define GFX_MODEL_H

#include "buffers.h"

struct Model {
	uint32 vertc;
	float* verts;
	VBO vbo;
	// struct Material material;
}; //static_assert(sizeof(struct Model) == 8, "struct Model");

struct Model create_model(char* const path);
void print_model(struct Model mdl, char* name);

#endif
