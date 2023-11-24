#ifndef GFX_RENDERER_H
#define GFX_RENDERER_H

#include <vulkan/vulkan.h>

#define FRAMES_IN_FLIGHT 2

void renderer_init();
void renderer_draw();
void renderer_free();

extern VkSampler default_sampler;
extern struct GlobalLighting {
	float ambient;
	float power;
	float p1[2];
	vec3  dir;
	float p2[1];
	vec3  colour;
	float p3[1];
} global_light;

#endif
