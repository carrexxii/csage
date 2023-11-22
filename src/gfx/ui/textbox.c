#include "gfx/primitives.h"
#include "ui.h"
#include "textbox.h"

void textbox_new(char* text, Rect rect, const struct UIStyle* style, struct UIObject* parent)
{
	// TODO: Add STRING_OF_UI()
	if (!parent || parent->type != UI_CONTAINER) {
		ERROR("[UI] Parent must be a valid UI_CONTAINER, got: %p", parent);
		return;
	}

	struct UIObject* obj = ui_alloc_object();
	obj->style  = style? style: &default_textbox_style;
	obj->type   = UI_TEXTBOX;
	obj->rect   = rect;
	obj->parent = parent;
	obj->z_lvl  = parent->z_lvl + 1;

	obj->textbox.text_obj = font_render(string_new(text, -1), obj->z_lvl + 10, 0); // TODO: fix z_lvl

	container_add(parent, obj);
	DEBUG(3, "[UI] Created new textbox %p with parent %p (%.2f, %.2f, %.2f, %.2f):\n\t%s",
	      obj, parent, rect.x, rect.y, rect.w, rect.h, text);
}

void textbox_build(struct UIObject* obj, struct VArray* verts)
{
	assert(obj && obj->type == UI_TEXTBOX);

	float points[6*UI_VERTEX_COUNT];
	Rect rect = ui_build_rect(obj, false);
	quad_from_rect(points, rect, (float)obj->z_lvl, obj->style->bg);
	varray_push_many(verts, 6, points);

	struct TextObject* txt_obj = obj->textbox.text_obj;
	switch (obj->style->align) {
	default:
	case ALIGN_CENTRE:
		txt_obj->rect.x =  rect.x + rect.w/2.0f - txt_obj->rect.w/2.0f;
		txt_obj->rect.y = -rect.y - rect.h/2.0f - txt_obj->rect.h/2.0f;
		break;
	case ALIGN_LEFT:
		txt_obj->rect.x =  rect.x;
		txt_obj->rect.y = -rect.y - txt_obj->rect.h;
		break;
	}

	obj->screen_rect = rect;
}
