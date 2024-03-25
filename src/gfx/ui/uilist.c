#include "types.h"
#include "ui.h"
#include "uilist.h"

void uilist_new(struct Container* parent, int strc, String* strs, Rect rect, bool numbered)
{
	assert(parent);

	struct UIObject obj = {
		.type    = UI_LIST,
		.rect    = ui_calc_rect(rect, parent),
		.state   = default_state,
		.imgi    = -1,
		.padding = VEC2(10.0f / config.winw, 5.0f / config.winh),
		.uilist = {
			.spacing   = 15.0f / config.winh,
			.text_objc = strc,
			.text_objs = smalloc(strc * sizeof(struct TextObject)),
		},
	};

	String str = string_new(NULL, 1024, NULL);
	float text_w = (obj.rect.w * config.winw / 2.0f) - 2.0f*obj.padding.x;
	for (int i = 0; i < strc; i++) {
		// TODO: make a string_ function
		if (numbered) {
			sprintf(str.data, "%d. ", i);
			str.len += strlen(str.data);
		}
		string_cat_cstr(&str, strs[i].data, -1);
		obj.uilist.text_objs[i] = font_render(str, UI_TEXT_Z_LVL, text_w);
		string_clear(&str);
	}
	string_free(&str);

	ui_add(parent, &obj);
	DEBUG(3, "[UI] Created new string list (%d strings) with parent %p (%.2f, %.2f, %.2f, %.2f)",
	      strc, parent, rect.x, rect.y, rect.w, rect.h);
}

void uilist_build(struct UIObject* obj, struct UIStyle* style)
{
	assert(obj->type == UI_LIST);
	style = style? style: &default_list_style;

	Rect rect = obj->rect;
	ui_update_object(obj->i, rect, obj->hl, style, obj->imgi, obj->state);

	struct TextObject* txt_obj;
	float y = (float)font_size / config.winh;
	for (int i = 0; i < obj->uilist.text_objc; i++) {
		txt_obj = obj->uilist.text_objs[i];
		txt_obj->rect.x =  rect.x + obj->padding.x;
		txt_obj->rect.y = -rect.y - obj->padding.y - y;
		y += txt_obj->rect.h + obj->uilist.spacing;
	}
}

bool uilist_on_hover(struct UIObject* obj, struct UIContext* context)
{
	assert(obj->type == UI_LIST);

	bool update = obj->state.hover || (obj->state.clicked == context->mouse_pressed.lmb);
	obj->state.hover   = true;
	obj->state.clicked = context->mouse_pressed.lmb;

	return update;
}

void uilist_on_click(struct UIObject* obj, struct UIContext* context)
{
	assert(obj->type == UI_LIST);

	obj->state.clicked = false;
	struct TextObject* txt_obj;
	Rect rect = RECT(obj->rect.x, obj->rect.y + 2.0f*obj->padding.y, obj->rect.w, 0.0f);
	for (int i = 0; i < obj->uilist.text_objc; i++) {
		txt_obj = obj->uilist.text_objs[i];
		rect.h = txt_obj->rect.h + obj->padding.y;
		if (point_in_rect(context->mouse_pos, rect)) {
			obj->hl = rect;
			uilist_build(obj, &default_label_style);
			break;
		}
		rect.y += txt_obj->rect.h + obj->uilist.spacing;
	}
}
