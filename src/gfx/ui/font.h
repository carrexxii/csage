#ifndef UI_FONT_H
#define UI_FONT_H

#include "vulkan/vulkan.h"

#include "util/string.h"
#include "gfx/buffers.h"

#define FONT_MAX_TEXT_OBJECTS 128

struct TextObject {
	Rect  rect;
	VBO   vbo;
	int   vertc;
	float z_lvl;
	bool  active;
};

void font_init(VkRenderPass renderpass);
struct TextObject* font_render(String text, float z, float w);
void font_record_commands(VkCommandBuffer cmdbuf);
void font_free();

extern int font_size;

#endif