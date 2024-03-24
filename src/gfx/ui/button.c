#include "maths/types.h"
#include "util/varray.h"
#include "gfx/primitives.h"
#include "types.h"
#include "ui.h"
#include "button.h"

void button_new(struct Container* parent, String text, struct Texture* tex, Rect rect, void (*cb)(void), struct UIStyle* style)
{
	if (!parent) {
		ERROR("[UI] Button must have a valid parent container");
		return;
	}

	Rect prect = parent->rect;
	float sx = prect.w / 2.0f;
	float sy = prect.h / 2.0f;
	struct UIObject obj = {
		.rect = RECT(rect.x*sx + prect.x + prect.w/2.0f,
		             rect.y*sy + prect.y + prect.h/2.0f,
		             rect.w*sx,
		             rect.h*sy),
		.type = UI_BUTTON,
		.button = {
			.cb = cb,
		},
	};
	if (text.len)
		obj.button.text_obj = font_render(text.data, text.len, UI_TEXT_Z_LVL, rect.w * config.winw);
	if (tex)
		parent->textures[parent->texturec++] = tex;
	varray_push(&parent->objects, &obj);

	DEBUG(3, "[UI] Created new button with parent %p (%.2f, %.2f, %.2f, %.2f): \"%s\" (image: %s)",
	      parent, rect.x, rect.y, rect.w, rect.h, text.data, STRING_TF(tex));
}

void button_build(struct UIObject* obj, struct VArray* verts, struct UIStyle* style)
{
	assert(obj->type == UI_BUTTON);

	Rect rect = obj->rect;
	Colour colour = obj->state.hover? style->fg: style->bg;
	varray_push_many(verts, 6, UI_VERTS(rect, colour));

	struct TextObject* txt_obj = obj->button.text_obj;
	txt_obj->rect.x =  rect.x + rect.w/2.0f - txt_obj->rect.w/2.0f;
	txt_obj->rect.y = -rect.y - rect.h/2.0f - txt_obj->rect.h/2.0f;
}

void button_on_click(struct UIObject* obj)
{
	D;
	obj->button.cb();
}
