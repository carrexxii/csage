#ifndef UI_CONTAINER_H
#define UI_CONTAINER_H

#include "util/arena.h"
#include "gfx/buffers.h"

#define CONTAINER_MAX_VERTS 256

struct Container {
	struct Arena* verts; /* Temporary scratch pad for adding vertices */
	VBO   vbo;
	usize vertc;
};

int  container_new(struct Rect rect, struct UIStyle* style, int parent);
void container_add(struct UIObject* obj, int ptc, float* pts);
void container_build(struct UIObject* obj);

#endif
