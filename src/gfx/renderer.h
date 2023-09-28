#ifndef GFX_RENDERER_H
#define GFX_RENDERER_H

#include <SDL2/SDL.h>

#define RENDERER_MAX_OBJECTS 256
#define FRAMES_IN_FLIGHT     2

void renderer_init();
void renderer_draw();
void renderer_free();

extern mat4* renmats;

#endif
