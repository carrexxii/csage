#ifndef UI_TEXTBOX_H
#define UI_TEXTBOX_H

#include "util/string.h"
#include "types.h"

void textbox_new(String text, Rect rect, struct UIStyle* style, struct UIObject* parent);
void textbox_build(struct UIObject* obj, struct VArray* verts);

#endif
