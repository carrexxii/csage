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
	obj->data = arena_alloc(ui_arena, sizeof(struct Container));
	*((struct Container*)obj->data) = (struct Container){ 0 };
	obj->style  = style? style: &default_container_style;
	obj->type   = UI_CONTAINER;
	obj->id     = ui_objc;
	obj->rect   = rect;
	obj->parent = parent < 0? -1: parent;
	obj->z_lvl  = obj->parent == -1? UI_BASE_Z_LEVEL
	                               : ui_objs[parent]->z_lvl + 1;

	if (parent != -1 && ui_objs[obj->parent]->type != UI_CONTAINER)
		ERROR("[UI] Container should not have parent other than another container");

	DEBUG(3, "[UI] Created new container %d with parent %d", obj->id, obj->parent);
	ui_objs[ui_objc++] = obj;
	return obj->id;
}

void container_add(struct UIObject* obj, int ptc, float* pts)
{
	struct Container* container = obj->data;
	assert(obj->type == UI_CONTAINER && container);
	if (!container->verts) {
		container->verts = arena_new(CONTAINER_MAX_VERTS*UI_VERTEX_SIZE, ARENA_NO_ALIGN);
		container->vertc = 0;
	}

	void* mem = arena_alloc(container->verts, ptc*UI_VERTEX_SIZE);
	memcpy(mem, pts, ptc*UI_VERTEX_SIZE);
	container->vertc += ptc;
}

/* NOTE: The vertex arena is freed after the vbo is built */
void container_build(struct UIObject* obj)
{
	struct Container* container = obj->data;
	assert(obj->type == UI_CONTAINER && container);
	if (!container->verts) {
		container->verts = arena_new(CONTAINER_MAX_VERTS*UI_VERTEX_SIZE, ARENA_NO_ALIGN);
		container->vertc = 0;
	}

	Rect  rect   = ui_build_rect(obj, false);
	void* points = arena_alloc(container->verts, 6*UI_VERTEX_SIZE);
	quad_points(points, rect, (float)obj->z_lvl, obj->style->bg);
	container->vertc += 6;
	container->vbo    = vbo_new(container->vertc*UI_VERTEX_SIZE, container->verts->data);

	DEBUG(3, "[UI] Container built with %lu vertices (z_lvl = %hhd)", container->vertc, obj->z_lvl);
	arena_free(container->verts);
	container->verts = NULL;
}
