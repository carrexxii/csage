#ifndef GFX_RENDERER_H
#define GFX_RENDERER_H

#include <vulkan/vulkan.h>

#define FRAMES_IN_FLIGHT 2

void renderer_init();
void renderer_draw();
void renderer_free();

extern VkSampler default_sampler;

#endif
