#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "util/string.h"
#include "types.h"

void button_new(struct UIContainer* parent, String text, struct Texture* img, Rect rect, void (*cb)(void));
void button_build(struct UIObject* obj, struct UIStyle* style);
bool button_on_hover(struct UIObject* obj, struct UIContext* context);
void button_on_click(struct UIObject* obj);

#endif
