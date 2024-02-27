#ifndef GFX_RENDERER_H
#define GFX_RENDERER_H

#include <vulkan/vulkan.h>

#include "maths/types.h"
#include "camera.h"

#define FRAMES_IN_FLIGHT            2
#define RENDERER_MAX_DRAW_FUNCTIONS 16

VkRenderPass renderer_init(void);
void renderer_clear_draw_list(void);
void renderer_add_to_draw_list(void (*fn)(VkCommandBuffer, struct Camera*));
void renderer_draw(struct Camera* cam);
void renderer_free(void);

extern VkSampler default_sampler;
extern struct GlobalLighting {
	Vec4 ambient; /* [colour|power] */
	Vec4 pos;     /* [pos|power]    */
	Vec3 colour;
} global_light;

#endif
