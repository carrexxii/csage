#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "gfx/font.h"

struct Button {
	struct TextObject text;
	void* fn_cb;
};

int button_new(Rect rect, struct UIStyle* style, int parent);
void button_build(struct UIObject* obj);

#endif
