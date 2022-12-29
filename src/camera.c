#include "config.h"
#include "camera.h"

#define MIN_ZOOM 40.0
#define MAX_ZOOM 250.0

mat4 camproj = GLM_MAT4_IDENTITY_INIT;
mat4 camview = GLM_MAT4_IDENTITY_INIT;
struct Rect    camrect;
enum Direction camdir;
int camzlvl;
int camzlvlmax   = 8;
int camzlvlscale = 8;
static float panspeed  = 5.0 * dt;
static float zoomspeed = 100.0 * dt;
static vec3 pos = { 0.0, 0.0, (MAX_ZOOM - MIN_ZOOM)/2.0 };

void camera_init()
{
	camera_set_perspective();
	camrect = (struct Rect){ 0 };

	versor q1, q2, q3;
	glm_mat4_copy(GLM_MAT4_IDENTITY, camview);
	glm_quat(q1,  glm_rad(180.0), 1.0, 0.0, 0.0);
	glm_quat(q2,  glm_rad(45.0), 0.0, 0.0, 1.0);
	glm_quat(q3, -glm_rad(45.0), 1.0, 0.0, 0.0); // atan(-1/sqrt(2.0))
	glm_quat_rotate(camview, q1, camview);
	glm_quat_rotate(camview, q2, camview);
	glm_quat_rotate(camview, q3, camview);
}

/* TODO: add conditional update of vp with boolean return + rename */
void camera_get_vp(mat4 out)
{
	camera_set_perspective();
	camview[0][3] = pos[0];
	camview[1][3] = pos[1];
	glm_mat4_mul(camview, camproj, out);
}

void camera_set_perspective()
{
	camrect.w = WINDOW_WIDTH  / pos[2];
	camrect.h = WINDOW_HEIGHT / pos[2];
	glm_ortho(-camrect.w/2.0, camrect.w/2.0, -camrect.h/2.0, camrect.h/2.0, 0.1, 1000.0, camproj);
}

void camera_update()
{
	if (camdir & DIR_RIGHT) pos[0] += -panspeed;
	if (camdir & DIR_LEFT)  pos[0] +=  panspeed;
	if (camdir & DIR_UP)    pos[1] +=  panspeed;
	if (camdir & DIR_DOWN)  pos[1] += -panspeed;
	if (camdir & DIR_FORWARDS)  pos[2] += zoomspeed;
	if (camdir & DIR_BACKWARDS) pos[2] -= zoomspeed;
	CLAMP(pos[2], MIN_ZOOM, MAX_ZOOM);
}
