#ifndef GFX_RENDERER_H
#define GFX_RENDERER_H

#include "common.h"
#include "maths/types.h"
#include "buffers.h"
#include "camera.h"

#define RENDERER_BACKGROUND_COLOUR  { 0.015, 0.015, 0.04, 0.0 }
#define FRAMES_IN_FLIGHT            2
#define RENDERER_MAX_DRAW_FUNCTIONS 16
#define RENDERER_MAX_LIGHTS         128

typedef struct DirectionalLight {
	Vec3 dir;      byte pad1[4];
	Vec3 ambient;  byte pad2[4];
	Vec3 diffuse;  byte pad3[4];
	Vec3 specular; byte pad4[4];
} DirectionalLight;

typedef struct PointLight {
	Vec3 pos;     byte pad1[4];
	Vec3 ambient; byte pad2[4];
	Vec3 diffuse; byte pad3[4];
	Vec3 specular;
	float constant;
	float linear;
	float quadratic;
	byte pad4[8];
} PointLight;

typedef struct SpotLight {
	Vec3 pos;     byte pad1[4];
	Vec3 dir;     byte pad2[4];
	Vec3 ambient; byte pad3[4];
	Vec3 diffuse; byte pad4[4];
	Vec3 specular;
	float constant;
	float linear;
	float quadratic;
	float cutoff;
	float outer_cutoff;
} SpotLight;

void renderer_init(void);
void renderer_clear_draw_list(void);
void renderer_add_to_draw_list(void (*fn)(VkCommandBuffer));
void renderer_set_global_lighting(Vec3 dir, Vec3 ambient, Vec3 diffuse, Vec3 specular);
void renderer_draw(void);
void renderer_free(void);

extern int64        frame_number;
extern VkRenderPass renderpass;
extern VkSampler    default_sampler;
extern UBO          global_camera_ubo;
extern UBO          global_light_ubo;

#endif

