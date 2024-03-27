#ifndef UI_TEXTBOX_H
#define UI_TEXTBOX_H

#include "util/string.h"
#include "types.h"

void label_new(struct UIContainer* parent, String str, struct UIStyle* style, Rect rect);
void label_build(struct UIObject* obj);

#endif
