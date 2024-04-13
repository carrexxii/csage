#ifndef UI_TEXTBOX_H
#define UI_TEXTBOX_H

#include "common.h"
#include "types.h"

void label_new(UIContainer* parent, String str, UIStyle* style, Rect rect);
void label_build(UIObject* obj);
void label_free(UIObject* obj);

#endif

