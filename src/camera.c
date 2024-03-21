#include "common.h"
#include "config.h"
#include "maths/la.h"
#include "maths/maths.h"
#include "input.h"
#include "camera.h"

// TODO: make camera the set-0 global uniform

struct Camera camera_new(Vec3 pos, Vec3 up, float w, float h, float fov)
{
	struct Camera cam = {
		.ubo        = ubo_new(sizeof(Mat4x4[2])),
		.pos        = pos,
		.up         = up,
		.pan_speed  = CAMERA_DEFAULT_PAN_SPEED,
		.zoom       = CAMERA_DEFAULT_ZOOM,
		.zoom_speed = CAMERA_DEFAULT_ZOOM_SPEED,
		.fov        = fov,
		.w          = w,
		.h          = h,
		.mats       = smalloc(sizeof(Mat4x4[2])),
	};

	camera_set_projection(&cam, CAMERA_ORTHOGONAL);
	cam.mats->view = translate_make(pos);
	camera_update(&cam);

	return cam;
}

void camera_set_projection(struct Camera* cam, enum CameraType type)
{
	float dist, sz_x, sz_y;
	cam->type = type;
	switch (type) {
	case CAMERA_ORTHOGONAL:
		CLAMP(cam->zoom, CAMERA_MIN_ZOOM, CAMERA_MAX_ZOOM);
		cam->mats->proj = cam_orthogonal(-16.0f / cam->zoom, 16.0f / cam->zoom,
		                                 -9.0f  / cam->zoom, 9.0f  / cam->zoom,
										 -1024.0f, 1024.0f);
		// dist = distance(VEC3_ZERO, cam->pos);
		// sz_x = (atan(cam->fov / 2.0f) * 2.0f) * dist;
		// sz_y = sz_x / (cam->w / cam->h);
		// cam->mats->proj = cam_orthogonal(-sz_x, sz_x, -sz_y, sz_y, -dist, 1024.0f);
		break;
	case CAMERA_PERSPECTIVE:
		cam->mats->proj = cam_perspective(cam->fov, cam->w / cam->h, 0.1f, 1024.0f);
		break;
	default:
		ERROR("Invalid camera type: %d", type);
		exit(1);
	}
}

void camera_move(struct Camera* cam, enum Direction dir, bool kdown)
{
	if (kdown)
		cam->dir |= dir;
	else
		cam->dir &= ~dir;
}

void camera_rotate(struct Camera* cam, enum Axis axis, float angle)
{
	switch (axis) {
	case AXIS_X: cam->pos = rotate(cam->pos, VEC3_X, angle); break;
	case AXIS_Y: cam->pos = rotate(cam->pos, VEC3_Y, angle); break;
	case AXIS_Z: cam->pos = rotate(cam->pos, VEC3_Z, angle); break;
	default: ERROR("[CAM] Invalid axis value: %d", axis);
	}
}

void camera_update(struct Camera* cam)
{
	Vec3 dir = { 0 };
	if (cam->dir & DIR_FORWARDS || cam->dir & DIR_BACKWARDS) {
		// dir = normalized(sub(cam->pos, VEC3_ZERO));
		// if (cam->dir & DIR_FORWARDS)
		// 	dir = multiply(dir, -0.1f);
		// else
		// 	dir = multiply(dir, 0.1f);
	}
	cam->pos = add(cam->pos, dir);

	if (cam->dir & DIR_FORWARDS)
		cam->zoom += cam->zoom_speed;
	if (cam->dir & DIR_BACKWARDS)
		cam->zoom -= cam->zoom_speed;

	if (cam->dir & DIR_LEFT) {
		cam->pos.x += cam->pan_speed / 2.0f;
		cam->pos.y -= cam->pan_speed / 2.0f;
		// cam->pos = rotate(cam->pos, cam->up, -deg_to_rad(1.0f));
	}
	if (cam->dir & DIR_RIGHT) {
		cam->pos.x -= cam->pan_speed / 2.0f;
		cam->pos.y += cam->pan_speed / 2.0f;
		// cam->pos = rotate(cam->pos, cam->up, deg_to_rad(1.0f));
	}
	if (cam->dir & DIR_UP) {
		cam->pos.y += cam->pan_speed / 2.0f;
		cam->pos.x += cam->pan_speed / 2.0f;
	}
	if (cam->dir & DIR_DOWN) {
		cam->pos.y -= cam->pan_speed / 2.0f;
		cam->pos.x -= cam->pan_speed / 2.0f;
	}
	// if (cam->dir & DIR_UP || cam->dir & DIR_DOWN) {
	// 	Vec3 pv = sub(cam->pos, VEC3_ZERO);
	// 	     pv = cross(cam->up, pv);
	// 	if (cam->dir & DIR_DOWN)
	// 		cam->pos = rotate(cam->pos, pv, -deg_to_rad(1.0f));
	// 	else
	// 		cam->pos = rotate(cam->pos, pv, deg_to_rad(1.0f));
	// }

	cam->mats->view = translate_make(cam->pos);
	// cam->mats->view = rotate(cam->mats->view, VEC3_Y,  deg_to_rad(180.0f));
	// cam->mats->view = rotate(cam->mats->view, VEC3_Z, -deg_to_rad(135.0f));
	// cam->mats->view = rotate(cam->mats->view, VEC3_X,  deg_to_rad(60.0f)); // 63.435f

	if (cam->type == CAMERA_ORTHOGONAL)
		camera_set_projection(cam, CAMERA_ORTHOGONAL);

	// TODO: conditional update
	buffer_update(cam->ubo, sizeof(Mat4x4[2]), (Mat4x4[]){ cam->mats->proj, cam->mats->view }, 0);
}

struct Ray camera_get_mouse_ray(struct Camera* cam, float x, float y)
{
	Mat4x4 cam_vp = multiply(cam->mats->proj, cam->mats->view);

	Vec3 p1 = unproject(VEC3(x, y, 0.0), cam_vp, RECT(0.0, 0.0, config.winw, config.winh));
	Vec3 p2 = unproject(VEC3(x, y, 1.0), cam_vp, RECT(0.0, 0.0, config.winw, config.winh));

	return ray_from_points(p1, p2);
}

Vec2 camera_get_map_point(struct Ray ray)
{
	Vec3 p = ray_plane_intersection(ray, VEC4(0.0, 0.0, -1.0, 0.0));

	return (Vec2){ p.x, p.y };
}
