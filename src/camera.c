#include "cglm/cglm.h"

#include "config.h"
#include "camera.h"

mat4 camProj = GLM_MAT4_IDENTITY_INIT;
mat4 camView = GLM_MAT4_IDENTITY_INIT;
struct Rect    camRect;
enum Direction camDir;
int camZlvl;
int camZlvlMax   = 8;
int camZlvlScale = 8;
static float camZoom      = 100.0;
static float panSpeed  = 0.2;
static float zoomSpeed = 3.0;
static float minZoom   = 2.0;
static float maxZoom   = 150.0;
mat4 camVP;

void init_cam()
{
	set_cam_perspective();
	camRect = (struct Rect){ 0 };
}

/* TODO: add conditional update of vp with boolean return + rename */
void get_cam_vp(mat4 out)
{
	set_cam_perspective();

	glm_mat4_copy(GLM_MAT4_IDENTITY, camView);
	glm_translate(camView, (vec3){ -camRect.x, -camRect.y + (float)(camZlvl*camZlvlScale - camZlvlScale), 0.0 });
	glm_rotate(camView, -GLM_PI*0.75, (vec3){ 0.0      , 0.0       , 1.0 });
	glm_rotate(camView,  GLM_PI/3.0 , (vec3){ GLM_SQRT2, -GLM_SQRT2, 0.0 });
	glm_scale_uni(camView, -1.0);

	glm_mul(camProj, camView, out);
}

void set_cam_perspective()
{
	camRect.w = 2.0 * 1600.0/camZoom;
	camRect.h = 2.0 *  900.0/camZoom;
	glm_ortho(-camRect.w/2.0, camRect.w/2.0, camRect.h/2.0, -camRect.h/2.0, 0.1, 1000.0, camProj);
}

void update_camera()
{
	if (camDir & DIR_UP)    camRect.y +=  panSpeed * (1.0/camZoom * maxZoom);
	if (camDir & DIR_DOWN)  camRect.y += -panSpeed * (1.0/camZoom * maxZoom);
	if (camDir & DIR_RIGHT) camRect.x +=  panSpeed * (1.0/camZoom * maxZoom);
	if (camDir & DIR_LEFT)  camRect.x += -panSpeed * (1.0/camZoom * maxZoom);
	if (camDir & DIR_FORWARDS)  camZoom += zoomSpeed;
	if (camDir & DIR_BACKWARDS) camZoom -= zoomSpeed;
	CLAMP(camZoom, minZoom, maxZoom);
}
