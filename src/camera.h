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
vec2s camera_get_point(float x, float y);
void  camera_set_perspective();
void  camera_update();

static void camera_move_up_cb(bool kdown)    { if (kdown) camdir |= DIRECTION_UP;    else camdir &= ~DIRECTION_UP;    }
static void camera_move_down_cb(bool kdown)  { if (kdown) camdir |= DIRECTION_DOWN;  else camdir &= ~DIRECTION_DOWN;  }
static void camera_move_right_cb(bool kdown) { if (kdown) camdir |= DIRECTION_RIGHT; else camdir &= ~DIRECTION_RIGHT; }
static void camera_move_left_cb(bool kdown)  { if (kdown) camdir |= DIRECTION_LEFT;  else camdir &= ~DIRECTION_LEFT;  }
static void camera_zoom_in_cb(bool kdown)    { if (kdown) camdir |= DIRECTION_FORWARDS;  else camdir &= ~DIRECTION_FORWARDS;  }
static void camera_zoom_out_cb(bool kdown)   { if (kdown) camdir |= DIRECTION_BACKWARDS; else camdir &= ~DIRECTION_BACKWARDS; }

inline static void camera_zlvl_up_cb(bool kdown)   { zoom += 1; CLAMP(zoom, 0, CAMERA_MAX_ZOOM); }
inline static void camera_zlvl_down_cb(bool kdown) { zoom -= 1; CLAMP(zoom, 0, CAMERA_MIN_ZOOM); }


#endif
