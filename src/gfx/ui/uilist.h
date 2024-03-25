#ifndef UI_LIST_H
#define UI_LIST_H

#include "types.h"

void uilist_new(struct Container* parent, int strc, String* strs, Rect rect, bool numbered);
void uilist_build(struct UIObject* obj, struct UIStyle* style);
bool uilist_on_hover(struct UIObject* obj, struct UIContext* context);
void uilist_on_click(struct UIObject* obj, struct UIContext* context);

#endif
