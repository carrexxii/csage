#include "common.h"
#include "config.h"
#include "camera.h"

mat4 camproj = GLM_MAT4_IDENTITY_INIT;
mat4 camview = GLM_MAT4_IDENTITY_INIT;
vec3 campos  = { 0.0, 0.0, 0.0 };
float zoom   = 100.0;
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
	glm_rotate_x(camview,  glm_rad(45.0) , camview);
	glm_rotate_z(camview, -glm_rad(135.0) , camview);

	glm_mat4_mul(camproj, camview, out);
}

vec2s camera_get_point(float x, float y)
{
	mat4 vp;
	camera_get_vp(vp);

	vec3s v = glms_unproject((vec3s){ x, y, 0.0 }, *(mat4s*)vp, (vec4s){ 0.0, 0.0, config_window_width, config_window_height });

	return (vec2s){ v.x, v.y };
}

void camera_set_perspective()
{
	glm_ortho(-(float)config_window_width/zoom , (float)config_window_width/zoom,
	          -(float)config_window_height/zoom, (float)config_window_height/zoom, 100.0, -100.0, camproj);
}

void camera_update()
{
	if (camdir & DIRECTION_RIGHT) campos[0] -= panspeed;
	if (camdir & DIRECTION_LEFT)  campos[0] += panspeed;
	if (camdir & DIRECTION_UP)    campos[1] -= panspeed;
	if (camdir & DIRECTION_DOWN)  campos[1] += panspeed;
	if (camdir & DIRECTION_FORWARDS)  zoom += zoomspeed;
	if (camdir & DIRECTION_BACKWARDS) zoom -= zoomspeed;
	CLAMP(zoom, CAMERA_MIN_ZOOM, CAMERA_MAX_ZOOM);
}
