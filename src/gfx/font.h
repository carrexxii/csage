#ifndef GFX_FONT_H
#define GFX_FONT_H

#include "vulkan/vulkan.h"

#include "buffers.h"

extern int font_size;

void font_init(VkRenderPass renderpass);
VBO  font_render(char* text, float x, float y, float maxw, float maxh);
void font_free();

#endif
