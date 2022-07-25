#ifndef GFX_CAMERA_H
#define GFX_CAMERA_H

#include "cglm/cglm.h"

enum Perspective {
	PERSPECTIVE_PERSPECTIVE,
	PERSPECTIVE_ORTHOGONAL,
};

extern mat4 camprojection;
extern mat4 camview;

void init_camera();
void get_cam_vp(mat4 out);
void set_perspective(enum Perspective p);

void move_camera(enum Direction dir);
inline static void move_camera_up_cb   (bool kdown) { move_camera(DIR_UP       ); }
inline static void move_camera_down_cb (bool kdown) { move_camera(DIR_DOWN     ); }
inline static void move_camera_right_cb(bool kdown) { move_camera(DIR_RIGHT    ); }
inline static void move_camera_left_cb (bool kdown) { move_camera(DIR_LEFT     ); }
inline static void zoom_camera_in_cb   (bool kdown) { move_camera(DIR_FORWARDS ); }
inline static void zoom_camera_out_cb  (bool kdown) { move_camera(DIR_BACKWARDS); }

#endif
