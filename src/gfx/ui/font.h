#ifndef UI_FONT_H
#define UI_FONT_H

#include <vulkan/vulkan.h>

#include "common.h"
#include "maths/maths.h"
#include "gfx/buffers.h"
#include "camera.h"

#define FONT_MAX_TEXT_OBJECTS 128

typedef struct TextObject {
	Rect  rect;
	VBO   vbo;
	int   vertc;
	float z_lvl;
	bool  active;
} TextObject;

void        font_init(void);
TextObject* font_render(String str, float z, float w);
void        font_record_commands(VkCommandBuffer cmd_buf);
void        font_free(void);

extern int font_size;

#endif

