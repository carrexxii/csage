#ifndef UI_TYPES_H
#define UI_TYPES_H

#include "common.h"
#include "maths/types.h"
#include "gfx/buffers.h"
#include "font.h"

#define UI_CONTAINER_MAX_IMAGES 8

typedef struct UIVertex {
	Vec2   pos;
	Colour colour;
} UIVertex;

typedef enum UIObjectType {
	UI_NONE,
	UI_CONTAINER,
	UI_BUTTON,
	UI_LABEL,
	UI_LIST,
	UI_CUSTOM,
} UIObjectType;

typedef struct UIState {
	bool visible;
	bool hover;
	bool clicked;
	bool dead;
} UIState;

typedef struct UIContext {
	Vec2 mouse_pos;
	struct { bool lmb, rmb; } mouse_pressed;
	struct { bool lmb, rmb; } mouse_released;
} UIContext;

typedef struct UIStyle {
	Colour normal;
	Colour hover;
	Colour clicked;
} UIStyle;

typedef struct UIContainer {
	Rect    rect;
	UIState state;
	bool    has_update;
	int     i;
	VArray  objects;
} UIContainer;

typedef struct UIShaderObject {
	Rect rect;
	Rect hl;
	Rect uv_rect;
	Vec4 colour;
	int  tex_id;
	byte pad[12];
} UIShaderObject;

typedef struct UIButton {
	TextObject* text_obj;
	void (*cb)(int);
	int data;
} UIButton;

typedef struct UILabel {
	TextObject* text_obj;
} UILabel;

typedef struct UIList {
	float        spacing;
	int          text_objc;
	TextObject** text_objs;
	void (*cb)(int);
} UIList;

struct UIObject;
typedef struct UICustom {
	void (*build)(struct UIObject*);
	bool (*on_hover)(struct UIObject*, struct UIContext*);
	void (*on_click)(struct UIObject*, struct UIContext*);
	void (*on_free)(struct UIObject*);
} UICustom;

typedef struct UIObject {
	Rect rect;
	Rect hl;
	Rect uv_rect;
	Vec2 padding;
	UIObjectType type;
	UIState state;
	int imgi;
	int i;
	UIStyle* style;
	union {
		UIButton button;
		UILabel  label;
		UIList   uilist;
		UICustom custom;
	};
} UIObject;

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

