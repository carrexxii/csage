#ifndef UI_LIST_H
#define UI_LIST_H

#include "common.h"
#include "types.h"

void uilist_new(UIContainer* parent, int strc, String* strs, UIStyle* style, Rect rect, bool numbered, void (*cb)(int));
void uilist_build(UIObject* obj);
bool uilist_on_hover(UIObject* obj, UIContext* context);
void uilist_on_click(UIObject* obj, UIContext* context);
void uilist_free(UIObject* obj);

#endif

