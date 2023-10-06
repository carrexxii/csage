#ifndef GFX_FONT_H
#define GFX_FONT_H

#include "vulkan/vulkan.h"

#include "buffers.h"

#define FONT_MAX_TEXT_OBJECTS 128

extern int font_size;

void font_init(VkRenderPass renderpass);
int  font_render(char* text, float x, float y, float maxw, float maxh);
void font_record_commands(VkCommandBuffer cmdbuf);
void font_free();

#endif
