#ifndef GFX_MODEL_H
#define GFX_MODEL_H

#include "buffers.h"

struct Material {
	vec3 rgb;
};

struct Model {
	VBO    vbo;
	float* verts; /* TODO: Remove this? */
	struct Material* materials; /* TODO: These don't need to be stored after mesh is loaded */
	vec3   dim;
	uint16 vertc;
	uint8  materialc;
};

struct Model model_new(char* path);
void model_free(struct Model* mdl);


#endif
