#ifndef GFX_FONT_H
#define GFX_FONT_H

#include "vulkan/vulkan.h"

#include "buffers.h"

#define FONT_MAX_TEXT_OBJECTS 128

extern int font_size;

void font_init(VkRenderPass renderpass);
int  font_render(char* text, float start_x, float start_y, float w);
void font_record_commands(VkCommandBuffer cmdbuf);
void font_free();

#endif