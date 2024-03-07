#ifndef GFX_RENDERER_H
#define GFX_RENDERER_H

#include "vulkan/vulkan.h"

#include "maths/types.h"
#include "buffers.h"
#include "camera.h"

#define RENDERER_MAX_DRAW_FUNCTIONS 16
#define RENDERER_MAX_LIGHTS         128

struct Light {
	Vec4 pos; /* w = power */
	Vec4 colour;
};

VkRenderPass renderer_init(void);
void renderer_clear_draw_list(void);
void renderer_add_to_draw_list(void (*fn)(VkCommandBuffer, struct Camera*));
void renderer_set_global_lighting(Vec3 light, float power, Vec3 ambient_colour, float ambient_power);
void renderer_draw(struct Camera* cam);
void renderer_free(void);

extern VkSampler default_sampler;
extern UBO       global_light_ubo;

#endif
