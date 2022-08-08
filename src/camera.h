#ifndef GFX_CAMERA_H
#define GFX_CAMERA_H

#include "cglm/cglm.h"

extern mat4 camprojection;
extern mat4 camview;
extern struct Rect camrect;
extern int camzlvl;
extern int camzlvlmax;
extern int camzlvlscale;

void init_cam();
void get_cam_vp(mat4 out);
void set_cam_perspective();

void move_cam(enum Direction dir);
static void move_cam_up_cb   (bool kdown) { move_cam(DIR_UP       ); }
static void move_cam_down_cb (bool kdown) { move_cam(DIR_DOWN     ); }
static void move_cam_right_cb(bool kdown) { move_cam(DIR_RIGHT    ); }
static void move_cam_left_cb (bool kdown) { move_cam(DIR_LEFT     ); }
static void zoom_cam_in_cb   (bool kdown) { move_cam(DIR_FORWARDS ); }
static void zoom_cam_out_cb  (bool kdown) { move_cam(DIR_BACKWARDS); }

inline static void cam_zlvl_up_cb  (bool kdown) { camzlvl += 1; CLAMP(camzlvl, 0, camzlvlmax); }
inline static void cam_zlvl_down_cb(bool kdown) { camzlvl -= 1; CLAMP(camzlvl, 0, camzlvlmax); }

#endif
