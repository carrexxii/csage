#ifndef GFX_MODEL_H
#define GFX_MODEL_H

#include <vulkan/vulkan.h>
#undef CGLTF_IMPLEMENTATION
#include "cgltf/cgltf.h"

#include "maths/maths.h"
#include "buffers.h"
#include "texture.h"
#include "camera.h"

// TODO: rename
#define MAX_MODELS             16
#define MAX_MATERIALS          8
#define MODEL_MAX_IMAGES       8
#define MODEL_MAX_JOINTS       48
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
	// struct Transform tform;
	float tform[16];
	float inv_bind[16];
	int   parent;
	float pad0[3];
};

struct Skin {
	SBO sbo;
	int jointc;
	float pad0;
	struct Joint joints[];
};

struct KeyFrame {
	struct Transform* tforms;
	float time;
};

struct ModelAnimation {
	// enum AnimationType     type;
	// enum InterpolationType interp;
	struct KeyFrame* frms;
	int frmc;
	int curr_frm;
};

// TODO: can make weights uint8
struct Model {
	struct Mesh*      meshes;
	struct Material*  mtls;
	struct ModelAnimation* anims;
	struct Skin*      skin;
	int meshc;
	int mtlc;
	int animc;
	int curr_anim;
	float timer;
};

void models_init();
struct Model* model_new(char* path);
void models_update();
void models_record_commands(VkCommandBuffer cmd_buf, struct Camera* cam);
void model_free(ID mdl_id);
void models_free();

extern void (*update_mdl_tforms)(SBO); // TODO: make this static
extern Mat4x4* mdl_tforms;

extern struct Material default_mtl;

#endif
