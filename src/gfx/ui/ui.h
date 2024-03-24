#ifndef UI_UI_H
#define UI_UI_H

#include "vulkan/vulkan.h"

#include "util/string.h"
#include "gfx/buffers.h"
#include "camera.h"
#include "types.h"
#include "font.h"
#include "container.h"
#include "button.h"
#include "textbox.h"

#define UI_MAX_CONTAINERS   8
#define UI_CONTAINER_Z_LVL  10
#define UI_OBJECT_Z_LVL     11
#define UI_TEXT_Z_LVL       12
#define UI_MAX_IMAGES       128
#define UI_DEFAULT_OBJECTS  8
#define UI_DEFAULT_VERTICES 128

#define UI_VERT(x, y, c) (struct UIVertex){ .pos = VEC2(x, y), .colour = c }
#define UI_VERTS(r, c) (struct UIVertex[]) { \
		UI_VERT(r.x + r.w, r.y      , c),    \
		UI_VERT(r.x      , r.y      , c),    \
		UI_VERT(r.x + r.w, r.y + r.h, c),    \
		UI_VERT(r.x      , r.y      , c),    \
		UI_VERT(r.x      , r.y + r.h, c),    \
		UI_VERT(r.x + r.w, r.y + r.h, c),    \
	}

void ui_init(void);
void ui_register_keys(void);
struct UIObject*  ui_alloc_object(void);
struct Container* ui_new_container(Rect rect, struct UIStyle* style);
void ui_add(struct Container* container, struct UIObject* obj);
void ui_build(void);
Rect ui_build_rect(struct UIObject* obj, bool absolute_sz);
void ui_update(void);
void ui_record_commands(VkCommandBuffer cmd_buf);
void ui_free(void);

#endif
