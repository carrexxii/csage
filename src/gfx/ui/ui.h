#ifndef UI_UI_H
#define UI_UI_H

#include <vulkan/vulkan.h>

#include "common.h"
#include "gfx/image.h"
#include "types.h"
#include "label.h"
#include "button.h"
#include "uilist.h"

#define UI_MAX_ELEMENTS     256
#define UI_MAX_CONTAINERS   8
#define UI_CONTAINER_Z_LVL  10
#define UI_OBJECT_Z_LVL     11
#define UI_TEXT_Z_LVL       12
#define UI_MAX_IMAGES       128
#define UI_DEFAULT_OBJECTS  8
#define UI_DEFAULT_VERTICES 128

#define ONE_PXX (1.0f / config.winw)
#define ONE_PXY (1.0f / config.winh)

void         ui_init(void);
void         ui_register_keys(void);
UIObject*    ui_alloc_object(void);
UIContainer* ui_new_container(Rect rect, UIStyle* style);
int          ui_add(UIContainer* container, UIObject* obj);
int          ui_add_image(Image* img);
void         ui_build(void);
Rect         ui_build_rect(UIObject* obj, bool absolute_sz);
void         ui_update(void);
void         ui_update_object(UIObject* obj);
void         ui_record_commands(VkCommandBuffer cmd_buf);
void         ui_free_container(UIContainer* container);
void         ui_free(void);

static inline Rect ui_calc_rect(Rect rect, UIContainer* parent)
{
	Rect prect = parent->rect;
	float sx = prect.w / 2.0f;
	float sy = prect.h / 2.0f;
	return RECT(
		rect.x*sx + prect.x + prect.w/2.0f,
		rect.y*sy + prect.y + prect.h/2.0f,
		rect.w*sx,
		rect.h*sy
	);
}

#endif

