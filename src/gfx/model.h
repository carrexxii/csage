#ifndef GFX_MODEL_H
#define GFX_MODEL_H

#include "cglm/cglm.h"

#include "buffers.h"

struct Model {
	VBO vbo;
	int vertc;
};

void model_free(struct Model* mdl);

#endif
