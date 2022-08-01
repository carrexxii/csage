#include "cglm/cglm.h"

#include "config.h"
#include "camera.h"
#include <cglm/mat4.h>
#include <cglm/types.h>
#include <cglm/vec3.h>

mat4 camprojection = GLM_MAT4_IDENTITY_INIT;
mat4 camview       = GLM_MAT4_IDENTITY_INIT;
static float camzoom = -1.0;
static float camfov  = GLM_PI/3.0;
static vec3 campos;
static vec3 lookpos;
static float panspeed  = 0.15;
static float zoomspeed = 0.1;
mat4 camvp;

void init_camera()
{
	set_perspective(PERSPECTIVE_ORTHOGONAL);
	glm_vec3_copy((vec3){ 0.0, 0.0, 0.0 }, campos);
	glm_vec3_copy((vec3){ 0.0, 0.0, 0.0 }, lookpos);
}

/* TODO: add conditional update of vp with boolean return */
void get_cam_vp(mat4 out)
{
	glm_mat4_copy(GLM_MAT4_IDENTITY, camview);
	glm_translate(camview, campos);
	glm_rotate(camview, -GLM_PI*0.75, (vec3){ 0.0      , 0.0       , 1.0 });
	glm_rotate(camview,  GLM_PI/3.0 , (vec3){ GLM_SQRT2, -GLM_SQRT2, 0.0 });
	glm_scale_uni(camview, camzoom);

	glm_mul(camprojection, camview, out);
}

void move_camera(enum Direction dir)
{
	vec3 vel = { 0.0, 0.0, 0.0 };
	switch (dir) {
		case DIR_UP   : vel[1] = -panspeed; break;
		case DIR_DOWN : vel[1] =  panspeed; break;
		case DIR_RIGHT: vel[0] =  panspeed; break;
		case DIR_LEFT : vel[0] = -panspeed; break;
		case DIR_FORWARDS : glm_vec3_scale(campos, 1.0 - zoomspeed, campos); return;
		case DIR_BACKWARDS: glm_vec3_scale(campos, 1.0 + zoomspeed, campos); return;
		default: ERROR("[INPUT] Invalid direction");
	}
	glm_vec3_add(vel, campos , campos);
	glm_vec3_add(vel, lookpos, lookpos);
}

void set_perspective(enum Perspective p)
{
	if (p == PERSPECTIVE_PERSPECTIVE)
		glm_perspective(camfov, 16.0/9.0, 0.1, 1000.0, camprojection);
	else if (p == PERSPECTIVE_ORTHOGONAL)
		glm_ortho(-16.0, 16.0, 9.0, -9.0, 0.1, 1000.0, camprojection);
	else
		ERROR("Unknown perspective %u", p);
}
