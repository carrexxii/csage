#ifndef GFX_CAMERA_H
#define GFX_CAMERA_H

#include "cglm/cglm.h"

extern mat4 camprojection;
extern mat4 camview;
extern struct Rect camrect;

void init_cam();
void get_cam_vp(mat4 out);
void set_cam_perspective();

void move_camera(enum Direction dir);
inline static void move_camera_up_cb   (bool kdown) { move_camera(DIR_UP       ); }
inline static void move_camera_down_cb (bool kdown) { move_camera(DIR_DOWN     ); }
inline static void move_camera_right_cb(bool kdown) { move_camera(DIR_RIGHT    ); }
inline static void move_camera_left_cb (bool kdown) { move_camera(DIR_LEFT     ); }
inline static void zoom_camera_in_cb   (bool kdown) { move_camera(DIR_FORWARDS ); }
inline static void zoom_camera_out_cb  (bool kdown) { move_camera(DIR_BACKWARDS); }

#endif
