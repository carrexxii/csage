#ifndef GFX_RENDERER_H
#define GFX_RENDERER_H

#include <SDL2/SDL.h>
#include "cglm/cglm.h"

#include "model.h"

#define RENDERER_MAX_OBJECTS 256
#define RENDERER_MAX_LIGHTS  8
#define FRAMES_IN_FLIGHT     2

void renderer_init();
void renderer_draw();
void renderer_add_light(vec4 light);
void renderer_set_lights(uint16 lightc, vec4* lights);
void renderer_free();

extern mat4* renmats;
extern uint16*       renmdlc;
extern struct Model* renmdls;

#endif
