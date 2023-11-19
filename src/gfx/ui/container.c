#include "util/arena.h"
#include "gfx/primitives.h"
#include "ui.h"
#include "container.h"

/* Passing `parent` as < 0 means no parent */
int container_new(Rect rect, struct UIStyle* style, int parent)
{
	if (!ui_is_valid_rect(rect))
		ERROR("[UI] Invalid rect passed for new container: (%.2f, %.2f, %.2f, %.2f)", rect.x, rect.y, rect.w, rect.h);
	if (parent >= (int)ui_objc)
		ERROR("[UI] Invalid parent object: %d (total objects: %d)", parent, ui_objc);

	struct UIObject* obj = arena_alloc(ui_arena, sizeof(struct UIObject));
	obj->data   = arena_alloc(ui_arena, sizeof(struct Container));
	obj->style  = style? style: &default_style;
	obj->rect   = rect;
	obj->type   = UI_CONTAINER;
	obj->id     = ui_objc;
	obj->parent = parent < 0? -1: parent;
	obj->z_lvl  = obj->parent == -1? 1: ui_objs[parent]->z_lvl + 1;
	obj->z_lvl = 0;

	ui_objs[ui_objc++] = obj;
	return obj->id;
}

void container_build(struct UIObject* obj)
{
	struct Container* container = obj->data;
	assert(obj->type == UI_CONTAINER && container);

	float points[4*7];
	quad_points(points, obj->rect, (float)obj->z_lvl, obj->style->bg);

	container->vbo   = vbo_new(sizeof(float[4*7]), points);
	container->vertc = 4;
}
