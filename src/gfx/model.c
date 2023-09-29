#include "model.h"
#include "buffers.h"

void model_free(struct Model* mdl)
{
	vbo_free(&mdl->vbo);
}
