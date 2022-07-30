#include "cglm/cglm.h"

#include "config.h"
#include "camera.h"

mat4 camprojection = GLM_MAT4_IDENTITY_INIT;
mat4 camview       = GLM_MAT4_IDENTITY_INIT;
static float camzoom = 1.0;
static float camfov  = GLM_PI/3.0;
static vec3 campos;
static vec3 camdir;
static float panspeed  = 0.15;
static float zoomspeed = 0.5;
mat4 camvp;

void init_camera()
{
	set_perspective(PERSPECTIVE_PERSPECTIVE);
}

/* TODO: add conditional update of vp with boolean return */
void get_cam_vp(mat4 out)
{
	glm_mat4_copy(GLM_MAT4_IDENTITY, camview);
	glm_lookat((vec3){0.0, 0.0, 10.0}, (vec3){0.0, 0.0, 0.0}, (vec3){0.0, 1.0, 0.0}, camview);
	glm_scale_uni(camview, camzoom);
	glm_translate(camview, campos);
	glm_rotate(camview, GLM_PI/4.0, (vec3){ 0.0,  0.0, 1.0 });
	glm_rotate(camview, GLM_PI/3.0, (vec3){ 0.5, -0.5, 0.0 });

	glm_mul(camprojection, camview, out);
}

void move_camera(enum Direction dir)
{
	vec3 vel = { 0.0, 0.0, 0.0 };
	switch (dir) {
		case DIR_UP   : vel[1] =  panspeed; break;
		case DIR_DOWN : vel[1] = -panspeed; break;
		case DIR_RIGHT: vel[0] = -panspeed; break;
		case DIR_LEFT : vel[0] =  panspeed; break;
		case DIR_FORWARDS : camzoom += zoomspeed; return;
		case DIR_BACKWARDS: camzoom -= zoomspeed; return;
		default: ERROR("[INPUT] Invalid direction");
	}
	glm_vec3_add(vel, campos, campos);
}

void set_perspective(enum Perspective p)
{
	if (p == PERSPECTIVE_PERSPECTIVE)
		glm_perspective(camfov, 16.0/9.0, 0.1, 100.0, camprojection);
	else if (p == PERSPECTIVE_ORTHOGONAL)
		glm_ortho(-16.0, 16.0,
		          -9.0, 9.0, 0.1, 100.0, camprojection);
}
