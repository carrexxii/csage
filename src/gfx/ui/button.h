#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "util/string.h"
#include "types.h"

void button_new(struct UIContainer* parent, Rect rect, String text, struct Texture* tex, Vec2 uvs[2], void (*cb)(int), int data, struct UIStyle* style);
void button_build(struct UIObject* obj);
bool button_on_hover(struct UIObject* obj, struct UIContext* context);
void button_on_click(struct UIObject* obj);

#endif
