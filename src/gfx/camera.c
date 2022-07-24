#include "cglm/cglm.h"

#include "config.h"
#include "camera.h"

mat4 camprojection = GLM_MAT4_IDENTITY_INIT;
mat4 camview       = GLM_MAT4_IDENTITY_INIT;
static float camzoom = 10.0;
static float camfov  = GLM_PI/2.0;
static vec3 campos;
static vec3 camdir;
mat4 camvp;

void init_camera()
{
	set_perspective(PERSPECTIVE_ORTHOGONAL);
	glm_scale_uni(camview, camzoom);
	glm_rotate(camview,  GLM_PI/4.0, (vec3){ 0.0, 0.0, 1.0 });
	glm_rotate(camview, -GLM_PI/3.0, (vec3){ 1.0, 0.0, 0.0 });
}

/* TODO: add conditional update of vp */
void get_cam_vp(mat4 out)
{
	glm_mat4_mul(camview, camprojection, out);
	glm_mat4_transpose(out);
	glm_mat4_print(out, stderr);
}

void set_perspective(enum Perspective p)
{
	if (p == PERSPECTIVE_PERSPECTIVE)
		glm_perspective(camfov, (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1, 100.0, camprojection);
	else if (p == PERSPECTIVE_ORTHOGONAL)
		glm_ortho(-WINDOW_WIDTH/20.0, WINDOW_WIDTH/20.0,
		          -WINDOW_HEIGHT/20.0, WINDOW_HEIGHT/20.0, 0.1, 100.0, camprojection);
}
