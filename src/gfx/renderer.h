#ifndef GFX_RENDERER_H
#define GFX_RENDERER_H

#include <SDL2/SDL.h>

#include "entities/model.h"

#define RENDERER_MAX_OBJECTS 256
#define FRAMES_IN_FLIGHT     2

void renderer_init(SDL_Window* win, uint16* mdlc, struct Model* mdls, uint16* matc, float* mats);
void renderer_draw();
void renderer_free();

#endif
