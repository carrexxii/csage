#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "util/string.h"
#include "util/varray.h"
#include "types.h"

void button_new(struct Container* parent, String text, struct Texture* img, Rect rect, void (*cb)(void), struct UIStyle* style);
void button_build(struct UIObject* obj, struct VArray* verts, struct UIStyle* style);
void button_on_click(struct UIObject* obj);

#endif
