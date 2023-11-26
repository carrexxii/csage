#ifndef GFX_RENDERER_H
#define GFX_RENDERER_H

#include <vulkan/vulkan.h>

#define FRAMES_IN_FLIGHT 2

VkRenderPass renderer_init();
void renderer_draw();
void renderer_free();

extern VkSampler default_sampler;
extern struct GlobalLighting {
	vec4 ambient; /* [colour|power] */
	vec4 pos;     /* [pos|power]    */
	vec3 colour;
} global_light;

#endif
