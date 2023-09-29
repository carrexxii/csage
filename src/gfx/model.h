#ifndef GFX_MODEL_H
#define GFX_MODEL_H

#include "buffers.h"

struct Model {
	int vertc;
	VBO vbo;
};

void model_free(struct Model* mdl);

#endif
