#ifndef GFX_RENDERER_H
#define GFX_RENDERER_H

#define RENDERER_MAX_OBJECTS 128
#define FRAMES_IN_FLIGHT     2

void renderer_init();
void renderer_draw();
void renderer_free();

#endif
