#include "gfx/primitives.h"
#include "types.h"
#include "ui.h"
#include "label.h"

void label_new(UIContainer* parent, String str, UIStyle* style, Rect rect)
{
	assert(parent);

	UIObject obj = {
		.type    = UI_LABEL,
		.style   = style? style: &default_label_style,
		.rect    = ui_calc_rect(rect, parent),
		.state   = default_state,
		.imgi    = -1,
		.padding = VEC2(10.0f / config.winw, 5.0f / config.winh),
	};

	float text_w = (obj.rect.w * config.winw / 2.0f) - 2.0f*obj.padding.x;
	obj.label.text_obj = font_render(str, UI_TEXT_Z_LVL, text_w);

	ui_add(parent, &obj);
	INFO(TERM_DARK_CYAN "[UI] Created new label with parent %p (%.2f, %.2f, %.2f, %.2f):\n\t\"%s\"",
	      (void*)parent, rect.x, rect.y, rect.w, rect.h, str.data);
}

void label_build(UIObject* obj)
{
	assert(obj->type == UI_LABEL);

	ui_update_object(obj);

	TextObject* txt_obj = obj->label.text_obj;
	txt_obj->rect.x =  obj->rect.x                   + obj->padding.x;
	txt_obj->rect.y = -obj->rect.y - txt_obj->rect.h - obj->padding.y;
}

void label_free(UIObject*)
{
	// font_free(obj->label.text_obj);
}

