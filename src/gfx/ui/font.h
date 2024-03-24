#ifndef UI_FONT_H
#define UI_FONT_H

#include "vulkan/vulkan.h"

#include "maths/maths.h"
#include "util/string.h"
#include "gfx/buffers.h"
#include "camera.h"

#define FONT_MAX_TEXT_OBJECTS 128

struct TextObject {
	Rect  rect;
	VBO   vbo;
	int   vertc;
	float z_lvl;
	bool  active;
};

void font_init(void);
struct TextObject* font_render(char* text, isize text_len, float z, float w);
void font_record_commands(VkCommandBuffer cmd_buf);
void font_free(void);

extern int font_size;

#endif
