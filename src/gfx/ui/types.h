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
	UI_TEXTBOX,
};

enum AlignType {
	ALIGN_NONE,
	ALIGN_LEFT,
	ALIGN_CENTRE,
	ALIGN_RIGHT,
};

// struct Container {
// 	struct VArray objs;
// 	struct VArray verts;
// 	VBO vbo;
// };

// struct Button {
// 	struct TextObject* text_obj;
// 	struct Rect screen_rect;
// 	void (*fn_cb)(void);
// };

// struct TextBox {
// 	struct TextObject* text_obj;
// 	String text;
// };

struct UIState {
	bool visible;
	bool hover;
	bool clicked;
	bool update;
};

struct UIMouse {
	bool lmb;
	bool rmb;
};

// struct UIObject {
// 	struct UIObject* parent;
// 	enum UIObjectType type;
// 	int8 z_lvl;

// 	struct UIState state;
// 	struct UIStyle* style;
// 	struct Rect rect;
// 	struct Texture img;
// 	union {
// 		struct Container container;
// 		struct Button    button;
// 		struct TextBox   textbox;
// 	};
// };

/////////////////////

struct Button {
	struct TextObject* text_obj;
	void (*cb)(void);
};

struct Label {
	struct TextObject* text_obj;
	String text;
};

struct UIObject {
	struct Rect rect;
	struct UIState state;
	enum UIObjectType type;
	union {
		struct Button button;
		struct Label  label;
	};
};

struct Container {
	Rect rect;
	struct UIState state;
	bool has_update;

	int texturec;
	struct Texture* textures[UI_CONTAINER_MAX_TEXTURES];
	struct VArray objects;
	VBO vbo;

	struct {
		struct UIStyle* container;
		struct UIStyle* button;
		struct UIStyle* label;
	} styles;
};

struct UIShaderObject {
	Rect   rect;
	Colour fg, bg;
};

////////////////////////

// TODO: Text buffering
struct UIContext {
	struct UIMouse mouse_pressed;
	struct UIMouse mouse_released;
	struct UIObject* clicked_obj;
	Vec2 mouse_pos;
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
	.bg = 0x55221155,
	.fg = 0xCCCCCCFF,
};
static struct UIStyle default_button_style = {
	.align = ALIGN_CENTRE,
	.bg = 0x664444FF,
	.fg = 0xCCCCCCFF,
};
static struct UIStyle default_label_style = {
	.align = ALIGN_LEFT,
	.bg = 0xAAAAAAFF,
	.fg = 0xCCCCCCFF,
};

#endif
