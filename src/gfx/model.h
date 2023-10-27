#ifndef GFX_MODEL_H
#define GFX_MODEL_H

#include "vulkan/vulkan.h"

#include "buffers.h"
#include "texture.h"

#define MAX_MODELS      16
#define MAX_MATERIALS   8
#define MAX_NAME_LENGTH 32

struct Mesh {
	VBO     vbos[5];
	IBO     ibo;
	// float*  verts;
	// uint16* inds;
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

struct Joint {
	struct Transform transform;
	char name[MAX_NAME_LENGTH];
	int  parent;
};

struct Skin {
	int jointc;
	struct Joint joints[];
};

struct Animation {
	char name[MAX_NAME_LENGTH];
	int framec;
	int bonec;
	struct Bone* bones;
	struct Transform** frame_transforms;
};

struct Model {
	struct Mesh*      meshes;
	struct Material*  materials;
	struct Animation* animations;
	struct Skin*      skin;
	int meshc;
	int materialc;
	int animationc;
};

void models_init(VkRenderPass render_pass);
struct Model* model_new(char* path, bool keep_verts);
void  models_record_commands(VkCommandBuffer cmd_buf);
mat4* model_get_matrix(ID model_id);
void  model_free(ID model_id);
void  models_free();

extern void (*update_model_transforms)(SBO);
extern mat4s* model_transforms;

#endif
