#include "config.h"
#include "util/varray.h"
#include "gfx/primitives.h"
#include "ui.h"
#include "button.h"

void button_new(String text, Rect rect, const struct UIStyle* style, struct UIObject* parent)
{
	// TODO: Add STRING_OF_UI()
	if (!parent || parent->type != UI_CONTAINER) {
		ERROR("[UI] Parent must be a valid UI_CONTAINER, got: %p", parent);
		return;
	}

	struct UIObject* obj = ui_alloc_object();
	obj->style  = style? style: &default_button_style;
	obj->type   = UI_BUTTON;
	obj->rect   = rect;
	obj->parent = parent;
	obj->z_lvl  = parent->z_lvl + 1;

	obj->button.text_obj = font_render(text.data, text.len, obj->z_lvl + 10, rect.w); // TODO: fix z_lvl
	obj->button.fn_cb = NULL;

	container_add(parent, obj);
	DEBUG(3, "[UI] Created new button %p with parent %p (%.2f, %.2f, %.2f, %.2f)",
	      obj, parent, rect.x, rect.y, rect.w, rect.h);
}

void button_build(struct UIObject* obj, struct VArray* verts)
{
	assert(obj && obj->type == UI_BUTTON);

	float points[6*UI_VERTEX_COUNT];
	Rect rect = ui_build_rect(obj, true);
	quad_from_rect(points, rect, (float)obj->z_lvl, obj->style->bg);
	varray_push_many(verts, 6, points);

	struct TextObject* txt_obj = obj->button.text_obj;
	txt_obj->rect.x =  rect.x + rect.w/2.0f - txt_obj->rect.w/2.0f;
	txt_obj->rect.y = -rect.y - rect.h/2.0f - txt_obj->rect.h/2.0f;

	obj->screen_rect = rect;
}

void button_on_hover(struct UIObject* obj)
{
	if (obj->state.hover) {
		obj->style = &default_container_style;
	} else {
		obj->style = &default_button_style;
	}
}
