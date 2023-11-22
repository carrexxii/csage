#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "util/string.h"
#include "util/varray.h"

struct Button {
	struct TextObject* text_obj;
	String text;
	void*  fn_cb;
};

void button_new(char* text, Rect rect, const struct UIStyle* style, struct UIObject* parent);
void button_build(struct UIObject* obj, struct VArray* verts);
void button_on_hover(struct UIObject* obj);

#endif
