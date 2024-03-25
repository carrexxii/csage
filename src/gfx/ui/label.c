#include "config.h"
#include "gfx/primitives.h"
#include "types.h"
#include "ui.h"
#include "label.h"

void label_new(struct Container* parent, String text, Rect rect)
{
	assert(parent);

	struct UIObject obj = {
		.type  = UI_LABEL,
		.rect  = ui_calc_rect(rect, parent),
		.state = default_state,
		.imgi  = -1,
	};

	float text_w = (obj.rect.w / parent->rect.w / 2.0f) * config.winw / 2.0f;
	obj.label.text_obj = font_render(text.data, text.len, UI_TEXT_Z_LVL, text_w);

	ui_add(parent, &obj);
	DEBUG(3, "[UI] Created new label with parent %p (%.2f, %.2f, %.2f, %.2f):\n\t%s",
	      parent, rect.x, rect.y, rect.w, rect.h, text.data);
}

void label_build(struct UIObject* obj, struct UIStyle* style)
{
	assert(obj->type == UI_LABEL);

	Rect rect = obj->rect;
	ui_update_object(obj->i, rect, style? style->bg: default_label_style.bg, obj->imgi, obj->state);

	struct TextObject* txt_obj = obj->label.text_obj;
	txt_obj->rect.x =  rect.x;
	txt_obj->rect.y = -rect.y - txt_obj->rect.h;
}
