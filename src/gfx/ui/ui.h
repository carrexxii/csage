#ifndef UI_UI_H
#define UI_UI_H

#include "vulkan/vulkan.h"

#include "util/string.h"
#include "gfx/buffers.h"
#include "input.h"
#include "container.h"
#include "button.h"

#define UI_MAX_TOP_LEVEL_CONTAINERS 8
#define UI_DEFAULT_OBJECT_COUNT     32
#define UI_BASE_Z_LEVEL             2
#define UI_VERTEX_COUNT             7
#define UI_VERTEX_SIZE              sizeof(float[UI_VERTEX_COUNT])

enum UIObjectType {
	UI_NONE,
	UI_CONTAINER,
	UI_BUTTON,
	UI_LABEL,
};

struct UIState {
	bool visible;
	bool hover;
};

struct UIObject {
	enum UIObjectType type;
	int8 z_lvl;

	struct Rect rect;
	struct Rect screen_rect;
	struct UIState state;
	const struct UIStyle* style;

	struct UIObject* parent;
	union {
		struct Container container;
		struct Button    button;
	};
};

// TODO: Text buffering
struct UIContext {
	enum MouseMask mouse_state;
	vec2 mouse_pos;
};

struct UIStyle {
	union Colour bg;
	union Colour fg;
};

static const struct UIStyle default_container_style = {
	.bg = 0x292929FF,
	.fg = 0xCCCCCCFF,
};
static const struct UIStyle default_button_style = {
	.bg = 0xFF0000FF,
	.fg = 0xCCCCCCFF,
};
extern struct UIContext ui_context;
extern struct UIObject  ui_containers[UI_MAX_TOP_LEVEL_CONTAINERS];
extern int ui_containerc;

/* -------------------------------------------------------------------- */

void ui_init(VkRenderPass renderpass);
struct UIObject* ui_alloc_object(void);
String ui_alloc_string(char* text, isize len);
void ui_build(void);
Rect ui_build_rect(struct UIObject* obj, bool absolute_sz);
void ui_update(void);
void ui_record_commands(VkCommandBuffer cmd_buf);
void ui_free(void);

#endif
