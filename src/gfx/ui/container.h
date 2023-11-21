#ifndef UI_CONTAINER_H
#define UI_CONTAINER_H

#include "util/arena.h"
#include "gfx/buffers.h"

#define CONTAINER_DEFAULT_VERTS 64
#define CONTAINER_DEFAULT_OBJS  8

struct UIStyle;
struct Container {
	struct VArray* objs;
	struct VArray* verts;
	VBO vbo;
};

struct UIObject* container_new(struct Rect rect, const struct UIStyle* style, struct UIObject* parent);
void container_add(struct UIObject* container_obj, struct UIObject* obj);
void container_build(struct UIObject* obj);
void container_free(struct UIObject* obj);

#endif
