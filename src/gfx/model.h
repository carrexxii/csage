#ifndef GFX_MODEL_H
#define GFX_MODEL_H

#include <vulkan/vulkan.h>
#undef CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "buffers.h"
#include "texture.h"

// TODO: rename
#define MAX_MODELS           16
#define MAX_MATERIALS        8
#define MODEL_MAX_IMAGES     8
#define MAX_JOINTS           64
#define MAX_NAME_LENGTH      32
#define MAX_ANIMATION_FRAMES 64
#define MODEL_MESH_VBO_COUNT 5

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
	int    materiali;
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
	struct Transform transform;
	char name[MAX_NAME_LENGTH];
	int  parent;
};

struct Skin {
	int jointc;
	struct Joint joints[];
};

struct AnimationFrame {
	struct Transform* transforms;
	int* joint_indices;
	int  jointc;
};

struct Animation {
	char name[MAX_NAME_LENGTH];
	enum AnimationType     type;
	enum InterpolationType interpolation;
	struct AnimationFrame* frames;
	float* times;
	int framec;
	int current_frame;
};

struct Model {
	struct Mesh*      meshes;
	struct Material*  materials;
	struct Animation* animations;
	struct Skin*      skin;
	int meshc;
	int materialc;
	int animationc;
	int current_animation;
	float timer;
};

void models_init(VkRenderPass renderpass);
struct Model* model_new(char* path);
void models_update();
void models_record_commands(VkCommandBuffer cmd_buf);
void model_free(ID model_id);
void models_free();

extern void (*update_model_transforms)(SBO);
extern mat4s* model_transforms;

extern struct Material default_material;

#endif
