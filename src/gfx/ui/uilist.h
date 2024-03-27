#ifndef UI_LIST_H
#define UI_LIST_H

#include "types.h"

void uilist_new(struct UIContainer* parent, int strc, String* strs, struct UIStyle* style, Rect rect, bool numbered, void (*cb)(int));
void uilist_build(struct UIObject* obj);
bool uilist_on_hover(struct UIObject* obj, struct UIContext* context);
void uilist_on_click(struct UIObject* obj, struct UIContext* context);

#endif
