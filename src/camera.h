#ifndef GFX_CAMERA_H
#define GFX_CAMERA_H

#include "map/map.h"

extern mat4 camproj;
extern mat4 camview;
extern struct Rect    camrect;
extern enum Direction camdir;
extern vec3 campos;
extern int camzlvl;

void camera_init();
void camera_get_vp(mat4 out);
void camera_unproject(float x, float y, vec3 out);
void camera_set_perspective();
void camera_update();

static void camera_move_up_cb   (bool kdown) { if (kdown) camdir |= DIR_UP;    else camdir &= ~DIR_UP;    }
static void camera_move_down_cb (bool kdown) { if (kdown) camdir |= DIR_DOWN;  else camdir &= ~DIR_DOWN;  }
static void camera_move_right_cb(bool kdown) { if (kdown) camdir |= DIR_RIGHT; else camdir &= ~DIR_RIGHT; }
static void camera_move_left_cb (bool kdown) { if (kdown) camdir |= DIR_LEFT;  else camdir &= ~DIR_LEFT;  }
static void camera_zoom_in_cb   (bool kdown) { if (kdown) camdir |= DIR_FORWARDS;  else camdir &= ~DIR_FORWARDS;  }
static void camera_zoom_out_cb  (bool kdown) { if (kdown) camdir |= DIR_BACKWARDS; else camdir &= ~DIR_BACKWARDS; }

// TODO: replace with cammaxzlvl
// inline static void camera_zlvl_up_cb  (bool kdown) { camzlvl += 1; CLAMP(camzlvl, 0, map->d - 1); }
// inline static void camera_zlvl_down_cb(bool kdown) { camzlvl -= 1; CLAMP(camzlvl, 0, map->d - 1); }

#endif

