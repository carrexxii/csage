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

#define UI_MAX_TOP_LEVEL_CONTAINERS 8
#define UI_DEFAULT_OBJECT_COUNT     32
#define UI_BASE_Z_LEVEL             2
#define UI_VERTEX_COUNT             7
#define UI_VERTEX_SIZE              sizeof(float[UI_VERTEX_COUNT])

extern struct UIContext ui_context;
extern struct UIObject  ui_containers[UI_MAX_TOP_LEVEL_CONTAINERS];
extern int ui_containerc;

/* -------------------------------------------------------------------- */

void ui_init(VkRenderPass renderpass);
struct UIObject* ui_alloc_object(void);
void ui_build(void);
Rect ui_build_rect(struct UIObject* obj, bool absolute_sz);
void ui_update(void);
void ui_record_commands(VkCommandBuffer cmd_buf, struct Camera* cam);
void ui_free(void);

#endif
