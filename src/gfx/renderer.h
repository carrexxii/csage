#ifndef GFX_RENDERER_H
#define GFX_RENDERER_H

#define FRAMES_IN_FLIGHT 2

extern struct ArrayList* renderermdls;

void renderer_init();
void renderer_draw();
void renderer_free();

#endif
