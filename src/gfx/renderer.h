#ifndef GFX_RENDERER_H
#define GFX_RENDERER_H

#include <SDL2/SDL.h>

#include "buffers.h"
#include "model.h"
#include "particles.h"

#define RENDERER_MAX_OBJECTS   256
#define RENDERER_MAX_PARTICLES 1024
#define FRAMES_IN_FLIGHT       2

void   renderer_init();
intptr renderer_add_model(struct Model mdl);
void   renderer_draw();
void   renderer_free();

extern mat4 renmats[RENDERER_MAX_OBJECTS];
extern intptr renmdlc;
extern struct Model renmdls[RENDERER_MAX_OBJECTS];
extern struct Particle renparticles[RENDERER_MAX_PARTICLES];

#endif
