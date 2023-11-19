#ifndef UI_CONTAINER_H
#define UI_CONTAINER_H

#include "util/arena.h"
#include "gfx/buffers.h"

struct Container {
	VBO   vbo;
	usize vertc;
};

int  container_new(Rect rect, struct UIStyle* style, int parent);
void container_build(struct UIObject* obj);

#endif
