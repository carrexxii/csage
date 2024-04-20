#ifndef GFX_MODEL_H
#define GFX_MODEL_H

#include <vulkan/vulkan.h>
#undef CGLTF_IMPLEMENTATION
#include "cgltf/cgltf.h"

#include "common.h"
#include "maths/maths.h"
#include "buffers.h"
#include "image.h"
#include "camera.h"

// TODO: rename
#define DEFAULT_MODEL_COUNT    8
#define DEFAULT_MATERIAL_COUNT 8
#define MODEL_MAX_JOINTS       64
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

typedef struct Mesh {
	VBO    vbos[MODEL_MESH_VBO_COUNT];
	IBO    ibo;
	int    vertc;
	uint16 indc;
	int    mtli;
} Mesh;

typedef struct Material {
	Vec4  albedo;
	float metallic;
	float roughness;
	Image img;
} Material;

typedef struct Joint {
	// struct Transform tform;
	float tform[16];
	float inv_bind[16];
	int   parent;
	float pad0[3];
} Joint;

typedef struct Skin {
	SBO sbo;
	int jointc;
	float pad0;
	Joint joints[];
} Skin;

typedef struct KeyFrame {
	Transform* tforms;
	float time;
} KeyFrame;

typedef struct ModelAnimation {
	// enum AnimationType     type;
	// enum InterpolationType interp;
	KeyFrame* frms;
	int frmc;
	int curr_frm;
} ModelAnimation;

typedef struct Model {
	isize meshc;
	Mesh* meshes;
	isize           animc;
	ModelAnimation* anims;
	Skin* skin;
	isize curr_anim;
	int64 timer;
} Model;

void  models_init(void);
void  model_update_pipelines(void);
isize model_new(const char* name);
void  models_update(void);
void  models_record_commands(VkCommandBuffer cmd_buf);
void  model_free(int mdl_id);
void  models_free(void);

#endif

