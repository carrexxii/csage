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

#endif
