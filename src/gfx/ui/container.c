#include "config.h"
#include "util/arena.h"
#include "gfx/primitives.h"
#include "ui.h"
#include "container.h"

int container_new(struct Rect rect, struct UIStyle* style, int parent)
{
	if (parent >= (int)ui_objc)
		ERROR("[UI] Invalid parent object: %d (total objects: %d)", parent, ui_objc);

	struct UIObject* obj = arena_alloc(ui_arena, sizeof(struct UIObject));
	obj->data   = arena_alloc(ui_arena, sizeof(struct Container));
	obj->style  = style? style: &default_style;
	obj->type   = UI_CONTAINER;
	obj->id     = ui_objc;
	obj->rect   = rect;
	obj->parent = parent < 0? -1: parent;
	obj->z_lvl  = obj->parent == -1? UI_BASE_Z_LEVEL
	                               : ui_objs[parent]->z_lvl + UI_BASE_Z_LEVEL;

	ui_objs[ui_objc++] = obj;
	return obj->id;
}

void container_build(struct UIObject* obj)
{
	struct Container* container = obj->data;
	assert(obj->type == UI_CONTAINER && container);

	float margin_x = (float)obj->style->margin / global_config.winw;
	float margin_y = (float)obj->style->margin / global_config.winh;

	float start_x, start_y;
	float scale_x, scale_y;
	if (obj->parent != -1) {
		struct Rect parent_rect = ui_objs[obj->parent]->rect;
		start_x = parent_rect.x + margin_x;
		start_y = parent_rect.y + margin_y;
		scale_x = parent_rect.w - 2.0f*margin_x;
		scale_y = parent_rect.h - 2.0f*margin_y;
	} else {
		start_x = 0.0f;
		start_y = 0.0f;
		scale_x = 1.0f;
		scale_y = 1.0f;
	}

	float points[4*7];
	struct Rect rect = RECT(start_x + obj->rect.x*scale_x + margin_x,
	                        start_y + obj->rect.y*scale_y + margin_y,
	                        obj->rect.w*scale_x - 2.0f*margin_x,
	                        obj->rect.h*scale_y - 2.0f*margin_y);
	quad_points(points, rect, (float)obj->z_lvl, obj->style->bg);

	container->vbo   = vbo_new(sizeof(float[4*7]), points);
	container->vertc = 4;
}
