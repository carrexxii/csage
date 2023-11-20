#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "util/string.h"
#include "gfx/font.h"

struct Button {
	String text;
	int    text_obj;
	void*  fn_cb;
};

int button_new(char* text, Rect rect, struct UIStyle* style, int parent);
void button_build(struct UIObject* obj);

#endif
