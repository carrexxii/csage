#ifndef UI_TEXTBOX_H
#define UI_TEXTBOX_H

#include "util/string.h"
#include "types.h"

void label_new(struct Container* parent, String text, Rect rect);
void label_build(struct UIObject* obj, struct UIStyle* style);

#endif
