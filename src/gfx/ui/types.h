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
	UI_LIST,
	UI_CUSTOM,
};

struct UIState {
	bool visible;
	bool hover;
	bool clicked;
};

struct UIContext {
	Vec2 mouse_pos;
	struct { bool lmb, rmb; } mouse_pressed;
	struct { bool lmb, rmb; } mouse_released;
};

struct UIStyle {
	Colour normal;
	Colour hover;
	Colour clicked;
};

struct UIContainer {
	Rect rect;
	struct UIState state;
	bool has_update;
	int i;
	struct VArray objects;
};

struct UIShaderObject {
	Rect rect;
	Rect hl;
	Vec4 colour;
	int  tex_id;
	byte pad[12];
};

struct UIButton {
	struct TextObject* text_obj;
	void (*cb)(void);
};

struct UILabel {
	struct TextObject* text_obj;
};

struct UIList {
	float spacing;
	int text_objc;
	struct TextObject** text_objs;
	void (*cb)(int);
};

struct UIObject;
struct UICustom {
	void (*build)(struct UIObject*);
	bool (*on_hover)(struct UIObject*, struct UIContext*);
	void (*on_click)(struct UIObject*, struct UIContext*);
};

struct UIObject {
	Rect rect;
	Rect hl;
	Vec2 padding;
	enum UIObjectType type;
	struct UIState state;
	int imgi;
	int i;
	struct UIStyle* style;
	union {
		struct UIButton button;
		struct UILabel  label;
		struct UIList   uilist;
		struct UICustom custom;
	};
};

/* -------------------------------------------------------------------- */

static struct UIState default_state = {
	.visible = true,
};

static struct UIStyle default_style = {
	.normal  = 0x888888FF,
	.hover   = 0xAAAAAAFF,
	.clicked = 0xCCCCCCFF,
};
static struct UIStyle default_container_style = {
	.normal  = 0x08083588,
};
static struct UIStyle default_button_style = {
	.normal  = 0x554585FF,
	.hover   = 0x9988AAFF,
	.clicked = 0xBFAFCFFF,
};
static struct UIStyle default_label_style = {
	.normal  = 0x888888FF,
	.hover   = 0xAAAAAAFF,
	.clicked = 0xCCCCCCFF,
};
static struct UIStyle default_list_style = {
	.normal  = 0x353535FF,
	.hover   = 0x353535FF,
	.clicked = 0x353535FF,
};

#endif
