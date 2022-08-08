#include "cglm/cglm.h"

#include "config.h"
#include "camera.h"
#include <cglm/mat4.h>
#include <cglm/types.h>
#include <cglm/vec3.h>

mat4 camprojection = GLM_MAT4_IDENTITY_INIT;
mat4 camview       = GLM_MAT4_IDENTITY_INIT;
struct Rect camrect;
static float zoom      = 100.0;
static float panspeed  = 0.2;
static float zoomspeed = 3.0;
static float minzoom   = 2.0;
static float maxzoom   = 150.0;
mat4 camvp;

void init_cam()
{
	set_cam_perspective();
	camrect = (struct Rect){ 0 };
}

/* TODO: add conditional update of vp with boolean return + rename */
void get_cam_vp(mat4 out)
{
	set_cam_perspective();

	glm_mat4_copy(GLM_MAT4_IDENTITY, camview);
	glm_translate(camview, (vec3){ camrect.x, camrect.y, 0.0 });
	glm_rotate(camview, -GLM_PI*0.75, (vec3){ 0.0      , 0.0       , 1.0 });
	glm_rotate(camview,  GLM_PI/3.0 , (vec3){ GLM_SQRT2, -GLM_SQRT2, 0.0 });
	glm_scale_uni(camview, -1.0);

	glm_mul(camprojection, camview, out);
}

void move_camera(enum Direction dir)
{
	vec3 vel = { 0 };
	switch (dir) {
		case DIR_UP   : vel[1] = -panspeed * (1.0/zoom * maxzoom); break;
		case DIR_DOWN : vel[1] =  panspeed * (1.0/zoom * maxzoom); break;
		case DIR_RIGHT: vel[0] = -panspeed * (1.0/zoom * maxzoom); break;
		case DIR_LEFT : vel[0] =  panspeed * (1.0/zoom * maxzoom); break;
		case DIR_FORWARDS : zoom += zoomspeed; CLAMP(zoom, minzoom, maxzoom); return;
		case DIR_BACKWARDS: zoom -= zoomspeed; CLAMP(zoom, minzoom, maxzoom); return;
		default: ERROR("[INPUT] Invalid direction");
	}
	camrect.x += vel[0];
	camrect.y += vel[1];
}

void set_cam_perspective()
{
	camrect.w = 2.0 * 1600.0/zoom;
	camrect.h = 2.0 *  900.0/zoom;
	glm_ortho(-camrect.w/2.0, camrect.w/2.0, camrect.h/2.0, -camrect.h/2.0, 0.1, 100.0, camprojection);
}
