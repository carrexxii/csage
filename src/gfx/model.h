#ifndef GFX_MODEL_H
#define GFX_MODEL_H

#include "vulkan/vulkan.h"

#include "buffers.h"

struct Mesh {
	VBO     vbo;
	IBO     ibo;
	float*  verts;
	uint16* inds;
	int     vertc;
	uint16  indc;
};

struct Model {
	struct Mesh* meshes;
	int meshc;
};

void  models_init(VkRenderPass render_pass);
ID    model_new(char* path, bool keep_verts);
void  models_record_commands(VkCommandBuffer cmd_buf);
mat4* model_get_matrix(ID model_id);
void  model_free(ID model_id);
void  models_free();

#endif
