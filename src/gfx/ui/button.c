#include "gfx/primitives.h"
#include "ui.h"
#include "button.h"

int button_new(Rect rect, struct UIStyle* style, int parent)
{
	if (!ui_is_valid_obj(parent))
		ERROR("[UI] Invalid parent for button: %d", parent);
	else if (ui_objs[parent]->type != UI_CONTAINER)
		ERROR("[UI] Parent for button should be a container object. Object %d is not a container", parent);

	struct UIObject* obj = arena_alloc(ui_arena, sizeof(struct UIObject));
	obj->data = arena_alloc(ui_arena, sizeof(struct Button));
	obj->style  = style? style: &default_button_style;
	obj->type   = UI_BUTTON;
	obj->id     = ui_objc;
	obj->rect   = rect;
	obj->parent = parent;
	obj->z_lvl  = obj->parent == -1? UI_BASE_Z_LEVEL
	                               : ui_objs[parent]->z_lvl + 1;
	// obj->z_lvl = ui_objs[parent]->z_lvl;

	if (ui_objs[obj->parent]->type != UI_CONTAINER)
		ERROR("[UI] Button was not given a container as parent");

	DEBUG(3, "[UI] Created new button %d with parent %d", obj->id, obj->parent);
	ui_objs[ui_objc++] = obj;
	return obj->id;
}

void button_build(struct UIObject* obj)
{
	struct Button* btn = obj->data;
	assert(obj->type == UI_BUTTON && btn);

	float points[6*7];
	Rect rect = ui_build_rect(obj, true);
	quad_points(points, rect, (float)obj->z_lvl, obj->style->bg);
	container_add(ui_objs[obj->parent], 6, points);

	DEBUG(5, "\tBuilt button %d on %d (z_lvl = %hhd)", obj->id, obj->parent, obj->z_lvl);
}
