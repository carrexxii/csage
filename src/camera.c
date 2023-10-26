#include "common.h"
#include "config.h"
#include "camera.h"
#include <cglm/mat4.h>

mat4 camproj = GLM_MAT4_IDENTITY_INIT;
mat4 camview = GLM_MAT4_IDENTITY_INIT;
vec3 campos  = { 0.0, 0.0, 0.0 };
float zoom   = 100.0;
enum Direction camdir = 0;
static float panspeed  = 6.0 * dt;
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
	glm_rotate_x(camview,  glm_rad(45.0), camview);
	glm_rotate_z(camview, -glm_rad(135.0), camview);

	glm_mat4_mul(camproj, camview, out);
}

struct Ray camera_get_mouse_ray(float x, float y)
{
	mat4 vp;
	camera_get_vp(vp);

	vec3 p1, p2;
	glm_unproject((vec3){ x, y, 0.0 }, vp, (float[]){ 0.0, 0.0, config_window_width, config_window_height }, p1);
	glm_unproject((vec3){ x, y, 1.0 }, vp, (float[]){ 0.0, 0.0, config_window_width, config_window_height }, p2);

	return ray_from_points(p1, p2);
}

vec2s camera_get_map_point(struct Ray ray)
{
	vec3 p;
	ray_plane_intersection(ray, (vec4){ 0.0, 0.0, -1.0, 0.0 }, p);

	return (vec2s){ p[0], p[1] };
}

void camera_set_perspective()
{
	glm_ortho((float)config_window_width/zoom , -(float)config_window_width/zoom,
	          (float)config_window_height/zoom, -(float)config_window_height/zoom, 1000.0, -16.0, camproj);
}

void camera_update()
{
	if (camdir & DIRECTION_RIGHT) campos[0] += panspeed;
	if (camdir & DIRECTION_LEFT)  campos[0] -= panspeed;
	if (camdir & DIRECTION_UP)    campos[1] -= panspeed;
	if (camdir & DIRECTION_DOWN)  campos[1] += panspeed;
	if (camdir & DIRECTION_FORWARDS)  zoom += zoomspeed;
	if (camdir & DIRECTION_BACKWARDS) zoom -= zoomspeed;
	CLAMP(zoom, CAMERA_MIN_ZOOM, CAMERA_MAX_ZOOM);
}
