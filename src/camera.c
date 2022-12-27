#include "config.h"
#include "camera.h"

mat4 camproj = GLM_MAT4_IDENTITY_INIT;
mat4 camview = GLM_MAT4_IDENTITY_INIT;
struct Rect    camrect;
enum Direction camdir;
int camzlvl;
int camzlvlmax   = 8;
int camzlvlscale = 8;
static float camzoom   = 100.0;
static float panspeed  = 5.0 * dt;
static float zoomspeed = 150.0 * dt;
static float minzoom   = 2.0;
static float maxzoom   = 150.0;

void camera_init()
{
	camera_set_perspective();
	camrect = (struct Rect){ 0 };
}

/* TODO: add conditional update of vp with boolean return + rename */
void camera_get_vp(mat4 out)
{
	camera_set_perspective();

	glm_mat4_copy(GLM_MAT4_IDENTITY, camview);

	glm_mat4_mul(camview, camproj, out);
}

void camera_set_perspective()
{
	camrect.w = WINDOW_WIDTH/camzoom;
	camrect.h = WINDOW_HEIGHT/camzoom;
	glm_ortho(-camrect.w/2.0, camrect.w/2.0, -camrect.h/2.0, camrect.h/2.0, 0.1, 1000.0, camproj);
}

void camera_update()
{
	if (camdir & DIR_UP)    camrect.x += -panspeed * (1.0/camzoom * maxzoom);
	if (camdir & DIR_DOWN)  camrect.x +=  panspeed * (1.0/camzoom * maxzoom);
	if (camdir & DIR_RIGHT) camrect.y +=  panspeed * (1.0/camzoom * maxzoom);
	if (camdir & DIR_LEFT)  camrect.y += -panspeed * (1.0/camzoom * maxzoom);
	if (camdir & DIR_FORWARDS)  camzoom += zoomspeed;
	if (camdir & DIR_BACKWARDS) camzoom -= zoomspeed;
	CLAMP(camzoom, minzoom, maxzoom);
}
