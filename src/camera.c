#include "config.h"
#include "camera.h"

mat4 camproj = GLM_MAT4_IDENTITY_INIT;
mat4 camview = GLM_MAT4_IDENTITY_INIT;
vec3 campos  = { 0.0, 0.0, 0.0 };
float zoom   = 300.0;
enum Direction camdir = 0;
static float panspeed  = 3.0 * dt;
static float zoomspeed = 100.0 * dt;

void camera_init()
{
	camera_set_perspective();
}

/* TODO: add conditional update of vp with boolean return + rename */
void camera_get_vp(mat4 out)
{
	camera_set_perspective();
	glm_translate_make(camview, campos);

	glm_mat4_mul(camproj, camview, out);
}

void camera_set_perspective()
{
	glm_ortho(-WINDOW_WIDTH/zoom, WINDOW_WIDTH/zoom, -WINDOW_HEIGHT/zoom, WINDOW_HEIGHT/zoom, 0.0, 100.0, camproj);
}

void camera_update()
{
	if (camdir & DIR_RIGHT) campos[0] -= panspeed;
	if (camdir & DIR_LEFT)  campos[0] += panspeed;
	if (camdir & DIR_UP)    campos[1] -= panspeed;
	if (camdir & DIR_DOWN)  campos[1] += panspeed;
	if (camdir & DIR_FORWARDS)  zoom += zoomspeed;
	if (camdir & DIR_BACKWARDS) zoom -= zoomspeed;
	CLAMP(zoom, CAMERA_MIN_ZOOM, CAMERA_MAX_ZOOM);
}
