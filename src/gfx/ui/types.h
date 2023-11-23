#ifndef UI_TYPES_H
#define UI_TYPES_H

#include "util/string.h"
#include "gfx/buffers.h"

enum UIObjectType {
	UI_NONE,
	UI_CONTAINER,
	UI_BUTTON,
	UI_TEXTBOX,
};

enum AlignType {
	ALIGN_NONE,
	ALIGN_LEFT,
	ALIGN_CENTRE,
	ALIGN_RIGHT,
};

struct Container {
	struct VArray* objs;
	struct VArray* verts;
	VBO vbo;
};

struct Button {
	struct TextObject* text_obj;
	struct Rect screen_rect;
	void (*fn_cb)(void);
};

struct TextBox {
	struct TextObject* text_obj;
	String text;
};

struct UIState {
	bool visible;
	bool hover;
	bool clicked;
};

struct UIMouse {
	bool lmb;
	bool rmb;
};

struct UIObject {
	struct UIObject* parent;
	enum UIObjectType type;
	int8 z_lvl;

	struct UIState state;
	struct UIStyle* style;
	struct Rect rect;
	union {
		struct Container container;
		struct Button    button;
		struct TextBox   textbox;
	};
};

// TODO: Text buffering
struct UIContext {
	struct UIMouse mouse_pressed;
	struct UIMouse mouse_released;
	struct UIObject* clicked_obj;
	vec2 mouse_pos;
};

struct UIStyle {
	enum AlignType align;
	union Colour bg;
	union Colour fg;
};

/* -------------------------------------------------------------------- */

static struct UIState default_state = {
	.visible = true,
	.hover   = false,
};

static struct UIStyle default_container_style = {
	.align = ALIGN_LEFT,
	.bg = 0xFF0000FF,
	.fg = 0xCCCCCCFF,
};
static struct UIStyle default_button_style = {
	.align = ALIGN_CENTRE,
	.bg = 0x00FF00FF,
	.fg = 0xCCCCCCFF,
};
static struct UIStyle default_textbox_style = {
	.align = ALIGN_LEFT,
	.bg = 0x0000FFFF,
	.fg = 0xCCCCCCFF,
};

#endif
