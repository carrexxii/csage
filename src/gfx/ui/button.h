#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "util/string.h"
#include "util/varray.h"
#include "types.h"

void button_new(String text, Rect rect, void (*fn_cb)(void), struct UIStyle* style, struct UIObject* parent);
void button_build(struct UIObject* obj, struct VArray* verts);
void button_on_click(struct UIObject* obj);

#endif
