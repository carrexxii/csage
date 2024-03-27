#include "maths/types.h"
#include "util/varray.h"
#include "gfx/primitives.h"
#include "types.h"
#include "ui.h"
#include "button.h"

void button_new(struct UIContainer* parent, String str, struct UIStyle* style, struct Texture* tex, Rect rect, void (*cb)(void))
{
	assert(parent);

	struct UIObject obj = {
		.type  = UI_BUTTON,
		.style = style? style: &default_button_style,
		.rect  = ui_calc_rect(rect, parent),
		.imgi  = -1,
		.state = default_state,
		.button = {
			.cb = cb,
		},
	};
	if (str.len)
		obj.button.text_obj = font_render(str, UI_TEXT_Z_LVL, rect.w * config.winw);
	if (tex)
		obj.imgi = ui_add_image(tex->image_view);
	ui_add(parent, &obj);

	DEBUG(3, "[UI] Created new button with parent %p (%.2f, %.2f, %.2f, %.2f): \"%s\" (image: %s)",
	      parent, rect.x, rect.y, rect.w, rect.h, str.data, STRING_TF(tex));
}

void button_build(struct UIObject* obj)
{
	assert(obj->type == UI_BUTTON);

	ui_update_object(obj);

	struct TextObject* txt_obj = obj->button.text_obj;
	txt_obj->rect.x =  obj->rect.x + obj->rect.w/2.0f - txt_obj->rect.w/2.0f;
	txt_obj->rect.y = -obj->rect.y - obj->rect.h/2.0f - txt_obj->rect.h/2.0f;
}

bool button_on_hover(struct UIObject* obj, struct UIContext* context)
{
	bool update = obj->state.hover || (obj->state.clicked == context->mouse_pressed.lmb);
	obj->state.hover   = true;
	obj->state.clicked = context->mouse_pressed.lmb;

	return update;
}

void button_on_click(struct UIObject* obj)
{
	obj->button.cb();
	obj->state.clicked = false;
}
