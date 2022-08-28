#include "config.h"
#include "camera.h"
#include "maths/matrix.h"

mat4 camProj = MAT4_IDENT;
mat4 camView = MAT4_IDENT;
struct Rect    camRect;
enum Direction camDir;
int camZlvl;
int camZlvlMax   = 8;
int camZlvlScale = 8;
static float camZoom   = 100.0;
static float panSpeed  = UPS(5.0);
static float zoomSpeed = UPS(150.0);
static float minZoom   = 2.0;
static float maxZoom   = 150.0;
mat4 camVP;

void init_cam()
{
	set_cam_perspective();
	camRect = (struct Rect){ 0 };
}

/* TODO: add conditional update of vp with boolean return + rename */
void get_cam_vp(mat4* out)
{
	set_cam_perspective();

	mat_copy(&camView, &MAT_I4);

	mat4 rot = MAT4_IDENT;
	vec4 q   = QUAT_IDENT;
	quat_mul_ip(&q, quat_new(1.0, 0.0, 0.0, -PI_4));
	quat_mul_ip(&q, quat_new(0.0, 0.0, 1.0, -PI_5));
	quat_to_matrix(q, &rot);
	mat_mul_ip(&camView, &rot);

	mat_translate(&camView, VEC3(-camRect.x, camRect.y, 0.0));

	mat_mul(out, &camView, &camProj);
}

void set_cam_perspective()
{
	camRect.w = 2.0 * WINDOW_WIDTH/camZoom;
	camRect.h = 2.0 * WINDOW_HEIGHT/camZoom;
	mat_new_ortho(&camProj, camRect.h/2.0, -camRect.h/2.0, camRect.w/2.0, -camRect.w/2.0, 0.1, 1000.0);
}

void update_camera()
{
	if (camDir & DIR_UP)    camRect.y += -panSpeed * (1.0/camZoom * maxZoom);
	if (camDir & DIR_DOWN)  camRect.y +=  panSpeed * (1.0/camZoom * maxZoom);
	if (camDir & DIR_RIGHT) camRect.x +=  panSpeed * (1.0/camZoom * maxZoom);
	if (camDir & DIR_LEFT)  camRect.x += -panSpeed * (1.0/camZoom * maxZoom);
	if (camDir & DIR_FORWARDS)  camZoom += zoomSpeed;
	if (camDir & DIR_BACKWARDS) camZoom -= zoomSpeed;
	CLAMP(camZoom, minZoom, maxZoom);
}
