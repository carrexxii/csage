#include "config.h"
#include "entities/entity.h"
#include "camera.h"

#define MIN_ZOOM 1.0
#define MAX_ZOOM 500.0

enum Direction camdir;
mat4 camproj   = GLM_MAT4_IDENTITY_INIT;
mat4 camview   = GLM_MAT4_IDENTITY_INIT;
vec3 campos    = { 0.0, 0.0, 0.0 };
int camzlvl    = 4;
int camzlvlmax = 8;
static float zoom      = 150.0;
static float panspeed  = 10.0 * dt;
static float zoomspeed = 50.0 * dt;

void camera_init()
{
	camera_set_perspective();
}

/* TODO: add conditional update of vp with boolean return + rename */
void camera_get_vp(mat4 out)
{
	camera_set_perspective();

	glm_translate_make(camview, campos);
	// glm_rotate_x(camview,  glm_rad(45.0), camview);
	// glm_rotate_z(camview, -glm_rad(45.0), camview);
	glm_rotate_x(camview,  glm_rad(180.0), camview);

	glm_mat4_mul(camproj, camview, out);
}

struct Ray camera_get_mouse_ray(float x, float y)
{
	mat4 vp;
	camera_get_vp(vp);

	vec3 p1, p2;
	glm_unproject((vec3){ x, y, 0.0 }, vp, (float[]){ 0.0, 0.0, WINDOW_WIDTH, WINDOW_HEIGHT }, p1);
	glm_unproject((vec3){ x, y, 1.0 }, vp, (float[]){ 0.0, 0.0, WINDOW_WIDTH, WINDOW_HEIGHT }, p2);

	return ray_from_points(p1, p2);
}

void camera_set_perspective()
{
	glm_ortho(-WINDOW_WIDTH/zoom, WINDOW_WIDTH/zoom, -WINDOW_HEIGHT/zoom, WINDOW_HEIGHT/zoom, 0.1, 100.0, camproj);
}

void camera_update()
{
	if (camdir & DIR_RIGHT) campos[0] -= panspeed;
	if (camdir & DIR_LEFT)  campos[0] += panspeed;
	if (camdir & DIR_UP)    campos[1] += panspeed;
	if (camdir & DIR_DOWN)  campos[1] -= panspeed;
	if (camdir & DIR_FORWARDS)  zoom += zoomspeed;
	if (camdir & DIR_BACKWARDS) zoom -= zoomspeed;
	CLAMP(zoom, MIN_ZOOM, MAX_ZOOM);
}

bool camera_select_entity_cb(int btn, bool btndown, int x, int y)
{
	if (!btndown)
		return false;

	return entity_select_by_ray(camera_get_mouse_ray(x, y));
}

