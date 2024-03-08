#ifndef GFX_RENDERER_H
#define GFX_RENDERER_H

#include "maths/types.h"
#include "buffers.h"
#include "camera.h"

#define RENDERER_MAX_DRAW_FUNCTIONS 16
#define RENDERER_MAX_LIGHTS         128

struct DirectionalLight {
	Vec3 dir;      float pad1;
	Vec3 ambient;  float pad2;
	Vec3 diffuse;  float pad3;
	Vec3 specular; float pad4;
};

struct PointLight {
	Vec3 pos;     float pad1;
	Vec3 ambient; float pad2;
	Vec3 diffuse; float pad3;
	Vec3 specular;
	float constant;
	float linear;
	float quadratic;
	float pad4[2];
};

struct SpotLight {
	Vec3 pos;     float pad1;
	Vec3 dir;     float pad2;
	Vec3 ambient; float pad3;
	Vec3 diffuse; float pad4;
	Vec3 specular;
	float constant;
	float linear;
	float quadratic;
	float cutoff;
	float outer_cutoff;
};

VkRenderPass renderer_init(void);
void renderer_clear_draw_list(void);
void renderer_add_to_draw_list(void (*fn)(VkCommandBuffer, struct Camera*));
void renderer_set_global_lighting(Vec3 dir, Vec3 ambient, Vec3 diffuse, Vec3 specular);
void renderer_draw(struct Camera* cam);
void renderer_free(void);

extern VkSampler default_sampler;
extern UBO       global_light_ubo;

#endif
