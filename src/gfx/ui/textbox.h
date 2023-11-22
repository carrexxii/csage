#ifndef UI_TEXTBOX_H
#define UI_TEXTBOX_H

#include "util/string.h"

struct TextBox {
	struct TextObject* text_obj;
	String text;
};

void textbox_new(char* text, Rect rect, const struct UIStyle* style, struct UIObject* parent);
void textbox_build(struct UIObject* obj, struct VArray* verts);

#endif
