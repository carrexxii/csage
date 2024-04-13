#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "common.h"
#include "gfx/image.h"
#include "types.h"

void button_new(UIContainer* parent, Rect rect, String text, Image* tex, Rect uv_rect, void (*cb)(int), int data, UIStyle* style);
void button_build(UIObject* obj);
bool button_on_hover(UIObject* obj, UIContext* context);
void button_on_click(UIObject* obj);
void button_free(UIObject* obj);

#endif

