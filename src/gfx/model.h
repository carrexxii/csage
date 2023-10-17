#ifndef GFX_MODEL_H
#define GFX_MODEL_H

#include "vulkan/vulkan.h"

#include "buffers.h"
#include "texture.h"

struct Mesh {
	VBO     vbo;
	IBO     ibo;
	float*  verts;
	uint16* inds;
	int     vertc;
	uint16  indc;
	int     materiali;
};

/* This must be aligned correctly for the shader */
struct Material {
	float albedo[4];
	float metallic;
	float roughness;
	struct Texture* texture;
};

struct Model {
	struct Mesh*     meshes;
	struct Material* materials;
	int meshc;
	int materialc;
};

void models_init(VkRenderPass render_pass);
struct Model* model_new(char* path, bool keep_verts);
void  models_record_commands(VkCommandBuffer cmd_buf);
mat4* model_get_matrix(ID model_id);
void  model_free(ID model_id);
void  models_free();

extern void (*update_model_transforms)(SBO);
extern mat4* model_transforms;

#endif
