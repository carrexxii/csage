#include "maths/types.h"
#include "util/varray.h"
#include "gfx/primitives.h"
#include "types.h"
#include "ui.h"
#include "button.h"

void button_new(struct Container* parent, String text, struct Texture* tex, Rect rect, void (*cb)(void))
{
	assert(parent);

	struct UIObject obj = {
		.type = UI_BUTTON,
		.rect = ui_calc_rect(rect, parent),
		.imgi = -1,
		.state = {
			.visible = true,
		},
		.button = {
			.cb = cb,
		},
	};
	if (text.len)
		obj.button.text_obj = font_render(text.data, text.len, UI_TEXT_Z_LVL, rect.w * config.winw);
	if (tex)
		obj.imgi = ui_add_image(tex->image_view);
	ui_add(parent, &obj);

	DEBUG(3, "[UI] Created new button with parent %p (%.2f, %.2f, %.2f, %.2f): \"%s\" (image: %s)",
	      parent, rect.x, rect.y, rect.w, rect.h, text.data, STRING_TF(tex));
}

void button_build(struct UIObject* obj, struct UIStyle* style)
{
	assert(obj->type == UI_BUTTON);

	Rect rect = obj->rect;
	ui_update_object(obj->i, rect, style? style->bg: default_button_style.bg, obj->imgi, obj->state);

	struct TextObject* txt_obj = obj->button.text_obj;
	txt_obj->rect.x =  rect.x + rect.w/2.0f - txt_obj->rect.w/2.0f;
	txt_obj->rect.y = -rect.y - rect.h/2.0f - txt_obj->rect.h/2.0f;
}

void button_on_click(struct UIObject* obj)
{
	obj->button.cb();
}
