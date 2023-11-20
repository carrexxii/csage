#ifndef GFX_CAMERA_H
#define GFX_CAMERA_H

#include "config.h"

#define CAMERA_MIN_ZOOM 0.0
#define CAMERA_MAX_ZOOM 500.0

extern mat4 camproj;
extern mat4 camview;
extern vec3 campos;
extern float zoom;
extern enum Direction camdir;

void  camera_init();
void  camera_get_vp(mat4 out);
struct Ray camera_get_mouse_ray(float x, float y);
vec2s camera_get_map_point(struct Ray ray);
void  camera_set_perspective();
void  camera_update();

#endif
