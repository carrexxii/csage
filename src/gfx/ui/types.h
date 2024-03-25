#ifndef UI_TYPES_H
#define UI_TYPES_H

#include "maths/types.h"
#include "util/string.h"
#include "util/varray.h"
#include "gfx/buffers.h"
#include "font.h"
#include "gfx/texture.h"

#define UI_CONTAINER_MAX_TEXTURES 8

struct UIVertex {
	Vec2   pos;
	Colour colour;
};

enum UIObjectType {
	UI_NONE,
	UI_CONTAINER,
	UI_BUTTON,
	UI_LABEL,
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

struct Button {
	struct TextObject* text_obj;
	void (*cb)(void);
};

struct Label {
	struct TextObject* text_obj;
};

struct UIObject {
	struct Rect rect;
	Vec2 padding;
	struct UIState state;
	int imgi;
	enum UIObjectType type;
	int i;
	union {
		struct Button button;
		struct Label  label;
	};
};

struct Container {
	Rect rect;
	struct UIState state;
	bool has_update;
	int i;
	struct VArray objects;
	struct {
		struct UIStyle* container;
		struct UIStyle* button;
		struct UIStyle* label;
	} styles;
};

struct UIShaderObject {
	Rect rect;
	Vec4 colour;
	int  tex_id;
	struct UIState state;
	byte pad[8];
};

struct UIContext {
	struct UIMouse mouse_pressed;
	struct UIMouse mouse_released;
	struct UIObject* clicked_obj;
	Vec2 mouse_pos;
};

struct UIStyle {
	union Colour bg;
	union Colour fg;
};

/* -------------------------------------------------------------------- */

static struct UIState default_state = {
	.visible = true,
};

static struct UIStyle default_container_style = {
	.bg = 0x55221155,
	.fg = 0xCCCCCCFF,
};
static struct UIStyle default_button_style = {
	.bg = 0x664444FF,
	.fg = 0xCCCCCCFF,
};
static struct UIStyle default_label_style = {
	.bg = 0xAAAAAAFF,
	.fg = 0xCCCCCCFF,
};

#endif
