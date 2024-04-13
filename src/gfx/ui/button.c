#include "maths/types.h"
#include "types.h"
#include "ui.h"
#include "button.h"

void button_new(UIContainer* parent, Rect rect, String text, Image* img, Rect uv_rect, void (*cb)(int), int data, UIStyle* style)
{
	assert(parent);

	UIObject obj = {
		.type    = UI_BUTTON,
		.style   = style? style: &default_button_style,
		.rect    = ui_calc_rect(rect, parent),
		.uv_rect = uv_rect,
		.imgi    = -1,
		.state   = default_state,
		.button = {
			.cb   = cb,
			.data = data,
		},
	};

	if (text.len)
		obj.button.text_obj = font_render(text, UI_TEXT_Z_LVL, rect.w * config.winw);
	if (img)
		obj.imgi = ui_add_image(img);
	ui_add(parent, &obj);

	INFO(TERM_DARK_CYAN "[UI] Created new button with parent %p (%.2f, %.2f, %.2f, %.2f): \"%s\" (image: %s)",
	      (void*)parent, rect.x, rect.y, rect.w, rect.h, text.data, STR_TF(img));
}

void button_build(UIObject* obj)
{
	assert(obj->type == UI_BUTTON);

	ui_update_object(obj);

	if (obj->button.text_obj) {
		TextObject* txt_obj = obj->button.text_obj;
		txt_obj->rect.x =  obj->rect.x + obj->rect.w/2.0f - txt_obj->rect.w/2.0f;
		txt_obj->rect.y = -obj->rect.y - obj->rect.h/2.0f - txt_obj->rect.h/2.0f;
	}
}

bool button_on_hover(UIObject* obj, UIContext* context)
{
	bool update = obj->state.hover || (obj->state.clicked == context->mouse_pressed.lmb);
	obj->state.hover   = true;
	obj->state.clicked = context->mouse_pressed.lmb;

	return update;
}

void button_on_click(UIObject* obj)
{
	obj->button.cb(obj->button.data);
	obj->state.clicked = false;
}

void button_free(UIObject*)
{
	// font_free(obj->button.text_obj);
}

