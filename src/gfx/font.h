#ifndef GFX_FONT_H
#define GFX_FONT_H

extern int font_size;

void font_init();
void font_render(void* target, int textlen, char* text, int x, int y);

#endif
