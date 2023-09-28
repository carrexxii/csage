#include "config.h"
#include "entities/entity.h"
#include "camera.h"

mat4 camproj   = GLM_MAT4_IDENTITY_INIT;
mat4 camview   = GLM_MAT4_IDENTITY_INIT;
vec3 campos    = { 10.0, 0.0, 0.0 };
enum Direction camdir = 0;
static float panspeed  = 10.0 * dt;
static float zoomspeed = 50.0 * dt;

void camera_init()
{
	glm_ortho(-WINDOW_WIDTH, WINDOW_WIDTH, -WINDOW_HEIGHT, WINDOW_HEIGHT, 0.1, 1000.0, camproj);
}

/* TODO: add conditional update of vp with boolean return + rename */
void camera_get_vp(mat4 out)
{
	glm_translate_make(camview, campos);

	glm_mat4_mul(camproj, camview, out);
}

void camera_update()
{
	if (camdir & DIR_RIGHT) campos[0] -= panspeed;
	if (camdir & DIR_LEFT)  campos[0] += panspeed;
	if (camdir & DIR_UP)    campos[1] += panspeed;
	if (camdir & DIR_DOWN)  campos[1] -= panspeed;
	if (camdir & DIR_FORWARDS)  campos[2] += zoomspeed;
	if (camdir & DIR_BACKWARDS) campos[2] -= zoomspeed;
	CLAMP(campos[2], CAMERA_MIN_ZOOM, CAMERA_MAX_ZOOM);
}
