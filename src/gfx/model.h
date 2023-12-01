#ifndef GFX_MODEL_H
#define GFX_MODEL_H

#include <vulkan/vulkan.h>
#undef CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "buffers.h"
#include "texture.h"

// TODO: rename
#define MAX_MODELS             16
#define MAX_MATERIALS          8
#define MODEL_MAX_IMAGES       8
#define MODEL_MAX_JOINTS       32
#define ANIMATION_MAX_NAME_LEN 32
#define JOINT_MAX_NAME_LEN     32
#define MAX_ANIMATION_FRAMES   64
#define MODEL_MESH_VBO_COUNT   5

enum InterpolationType {
	INTERPOLATION_LINEAR       = cgltf_interpolation_type_linear,
	INTERPOLATION_STEP         = cgltf_interpolation_type_step,
	INTERPOLATION_CUBIC_SPLINE = cgltf_interpolation_type_cubic_spline,
};

enum AnimationType {
	ANIMATION_TRANSLATE = cgltf_animation_path_type_translation,
	ANIMATION_SCALE     = cgltf_animation_path_type_scale,
	ANIMATION_ROTATE    = cgltf_animation_path_type_rotation,
};

struct Mesh {
	VBO    vbos[MODEL_MESH_VBO_COUNT];
	IBO    ibo;
	int    vertc;
	uint16 indc;
	int    mtli;
};

/* This must be aligned correctly for the shader */
struct Material {
	float albedo[4];
	float metallic;
	float roughness;
	struct Texture tex;
	VkDescriptorSet dset;
	float pad0[2];
};

struct Joint {
	struct Transform tform;
	char name[JOINT_MAX_NAME_LEN]; // TODO: move as array to skin so it can be cleared when not debugging
	int  parent;
};

struct Skin {
	SBO sbo;
	int jointc;
	struct Joint joints[];
};

struct KeyFrame {
	struct Transform* tforms;
	// uint8* joints;
	// uint8  jointc;
	float  time;
};

struct Animation {
	char name[ANIMATION_MAX_NAME_LEN]; // TODO: same as for joint names
	// enum AnimationType     type;
	// enum InterpolationType interp;
	struct KeyFrame* frms;
	// float* times;
	int frmc;
	int curr_frm;
};

struct Model {
	struct Mesh*      meshes;
	struct Material*  mtls;
	struct Animation* anims;
	struct Skin*      skin;
	int meshc;
	int mtlc;
	int animc;
	int curr_anim;
	float timer;
};

void models_init(VkRenderPass renderpass);
struct Model* model_new(char* path);
void models_update();
void models_record_commands(VkCommandBuffer cmd_buf);
void model_free(ID mdl_id);
void models_free();

extern void (*update_mdl_tforms)(SBO); // TODO: make this static
extern mat4s* mdl_tforms;

extern struct Material default_mtl;

#endif
