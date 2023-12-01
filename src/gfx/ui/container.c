#include "config.h"
#include "util/varray.h"
#include "gfx/primitives.h"
#include "types.h"
#include "ui.h"
#include "container.h"
#include "textbox.h"

struct UIObject* container_new(struct Rect rect, struct UIStyle* style, struct UIObject* parent)
{
	// TODO: STRING_OF_UI_TYPE()
	if (parent && parent->type != UI_CONTAINER) {
		ERROR("[UI] Invalid parent object: %p (parent for container must be a containerm not: %d)", parent, parent->type);
		return NULL;
	}

	struct UIObject* obj;
	if (parent) {
		obj = ui_alloc_object();
	} else {
		if (ui_containerc >= UI_MAX_TOP_LEVEL_CONTAINERS) {
			ERROR("[UI] Exceeded maximum number of top-level containers (%d)", UI_MAX_TOP_LEVEL_CONTAINERS);
			return NULL;
		}
		obj = &ui_containers[ui_containerc++];
	}
	obj->style  = style? style: &default_container_style;
	obj->type   = UI_CONTAINER;
	obj->rect   = rect;
	obj->parent = parent;
	obj->z_lvl  = parent? parent->z_lvl + 1: UI_BASE_Z_LEVEL;
	obj->state  = default_state;

	obj->container.verts = varray_new(CONTAINER_DEFAULT_VERTS, UI_VERTEX_SIZE);
	obj->container.objs  = varray_new(CONTAINER_DEFAULT_OBJS , sizeof(struct UIObject*));
	obj->container.vbo   = vbo_new(CONTAINER_DEFAULT_VERTS*UI_VERTEX_SIZE, obj->container.verts->data, true);

	DEBUG(3, "[UI] Created new container %p with parent %p (%.2f, %.2f, %.2f, %.2f)",
	      obj, obj->parent, rect.x, rect.y, rect.w, rect.h);
	return obj;
}

void container_add(struct UIObject* container_obj, struct UIObject* obj)
{
	assert(container_obj != obj && container_obj->type == UI_CONTAINER && obj->type != UI_CONTAINER);

	varray_push(container_obj->container.objs, &obj);
}

void container_build(struct UIObject* obj)
{
	assert(obj && obj->type == UI_CONTAINER);

	struct VArray* verts = obj->container.verts;
	struct VArray* objs  = obj->container.objs;
	varray_reset(verts);

	float points[6*UI_VERTEX_COUNT];
	Rect rect = ui_build_rect(obj, false);
	quad_from_rect(points, rect, (float)obj->z_lvl, obj->style->bg);
	varray_push_many(verts, 6, points);
	struct UIObject* inner_obj;
	for (int i = 0; i < objs->len; i++) {
		inner_obj = *(struct UIObject**)varray_get(objs, i);
		switch (inner_obj->type) {
		case UI_BUTTON : button_build(inner_obj, verts);  break;
		case UI_TEXTBOX: textbox_build(inner_obj, verts); break;
		default:
			// TODO: STRING_OF_UI()
			ERROR("[UI] Unrecognized type for UIObject %p: %d", inner_obj, inner_obj->type);
			exit(1);
		}
	}

	// TODO: Check for resizing vbo (when varray resizes)
	buffer_update(obj->container.vbo, verts->len*UI_VERTEX_SIZE, verts->data, 0);
}

void container_free(struct UIObject* obj)
{
	vbo_free(&obj->container.vbo);
}
