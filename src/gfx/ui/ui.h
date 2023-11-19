#ifndef UI_UI_H
#define UI_UI_H

#include "vulkan/vulkan.h"

#include "util/string.h"
#include "gfx/buffers.h"

#define UI_MAX_OBJECTS        64
#define UI_ARENA_DEFAULT_SIZE 4096
#define UI_BASE_Z_LEVEL       2

enum UIObjectType {
	UI_NONE,
	UI_CONTAINER,
	UI_BUTTON,
	UI_LABEL,
};

struct UIState {
	bool visible;
	bool focus;
};

struct UIObject {
	int id;
	int parent;
	enum UIObjectType type;
	int8  z_lvl;
	void* data;

	struct Rect rect;
	struct UIState  state;
	struct UIStyle* style;
};

enum MouseState {
	MOUSE_STATE_NONE       = 0x00,
	MOUSE_STATE_LMB_DOWN   = 0x01,
	MOUSE_STATE_RMB_DOWN   = 0x02,
	MOUSE_STATE_MMB_DOWN   = 0x04,
	MOUSE_STATE_MOUSE_DRAG = 0x08,
};

// TODO: Text buffering
struct UIContext {
	enum MouseState mouse_state;
	vec2 mouse_pos;
};

struct UIStyle {
	String* title;
	uint8   padding;
	uint8   margin;
	union Colour bg;
	union Colour fg;
};

static struct UIStyle default_style = {
	.title  = NULL,
	.margin = 10,
	.bg = 0x0000FFFF,
	.fg = 0xCCCCCCFF,
};
static struct UIStyle test_style = {
	.title  = NULL,
	.margin = 10,
	.bg = 0xFF00FFFF,
	.fg = 0xDDDDDDFF,
};
extern struct Arena* ui_arena;
extern struct UIContext ui_context;
extern struct UIObject* ui_objs[UI_MAX_OBJECTS];
extern int ui_objc;

#include "container.h"

/* -------------------------------------------------------------------- */

void ui_init(VkRenderPass renderpass);
bool ui_is_valid_rect(Rect rect);
void ui_build(void);
void ui_record_commands(VkCommandBuffer cmd_buf);
void ui_free(void);

#endif
