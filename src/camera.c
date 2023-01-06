#include "config.h"
#include "camera.h"

#define MIN_ZOOM 1.0
#define MAX_ZOOM 500.0

mat4 camproj = GLM_MAT4_IDENTITY_INIT;
mat4 camview = GLM_MAT4_IDENTITY_INIT;
struct Rect    camrect;
enum Direction camdir;
vec3 campos = { 0.0, 0.0, 0.0 };
int camzlvl;
int camzlvlmax   = 8;
int camzlvlscale = 8;
static float zoom = 150.0;
static float panspeed  = 5.0 * dt;
static float zoomspeed = 50.0 * dt;

void camera_init()
{
	camera_set_perspective();
	camrect = (struct Rect){ 0 };
}

/* TODO: add conditional update of vp with boolean return + rename */
void camera_get_vp(mat4 out)
{
	camera_set_perspective();

	glm_translate_make(camview, campos);
	glm_rotate_x(camview,  glm_rad(45.0), camview); // camera_unproject() will need to be
	glm_rotate_z(camview, -glm_rad(45.0), camview); // changed if these angles are changed
	glm_rotate_x(camview,  glm_rad(180.0), camview);
	// glm_scale(camview, (vec3){ zoom, zoom, zoom });

	glm_mat4_mul(camproj, camview, out);
}

void camera_unproject(float x, float y, vec3 out)
{
	mat4 vp;
	camera_get_vp(vp);
	glm_unproject((vec3){ x, y, 0.0 }, vp, (float[]){ 0.0, 0.0, WINDOW_WIDTH, WINDOW_HEIGHT }, out);
	glm_vec3_add(out, (vec3){ 0.5*(out[0] + out[1]), 0.5*(out[0] + out[1]), -out[2] }, out); // idk, isometric correction
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

