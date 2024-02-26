#include "config.h"
#include "maths/la.h"
#include "maths/maths.h"
#include "input.h"
#include "camera.h"

struct Camera camera_new(Vec3 pos, Vec3 up, float w, float h, float fov)
{
	struct Camera cam = {
		.pos  = pos,
		.up   = up,
		.fov  = fov,
		.w    = w,
		.h    = h,
		.mats = smalloc(sizeof(Mat4x4[2])),
	};

	cam.mats->proj = MAT4X4_IDENTITY;

	// glm_translate_make(cam.mats->view.m, pos.v);
	camera_update(&cam);

	return cam;
}

void camera_set_projection(struct Camera* cam, enum CameraType type)
{
	float dist, sz_x, sz_y;
	cam->type = type;
	switch (type) {
	case CAMERA_ORTHOGONAL:
		dist = distance(VEC3_ZERO, cam->pos);
		sz_x = (atan(cam->fov / 2.0f) * 2.0f) * dist;
		sz_y = sz_x / (cam->w/cam->h);
		// glm_ortho(-sz_x, sz_x, -sz_y, sz_y,
		//           -dist, 1024.0f, cam->mats->proj.m);
		break;
	case CAMERA_PERSPECTIVE:
		// glm_perspective(cam->fov, cam->w / cam->h,
		//                 0.1f, 1024.0f, cam->mats->proj.m);
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

void camera_update(struct Camera* cam)
{
	Vec3 dir = { 0 };
	if (cam->dir & DIR_FORWARDS || cam->dir & DIR_BACKWARDS) {
		dir = normalized(sub(cam->pos, VEC3(0.0f, 0.0f, 0.0f)));
		if (cam->dir & DIR_FORWARDS)
			dir = multiply(dir, -0.1f);
		else
			dir = multiply(dir, 0.1f);
	}
	cam->pos = add(cam->pos, dir);
	add(VEC3(1,2,3), 1.0f);
	add(VEC3(1,2,3), VEC3(1,2,3));
	Vec3 p = VEC3(1,2,3);
	Vec3 o = VEC3(1,2,3);
	add(p, o);

	if (cam->dir & DIR_LEFT)
		;// glm_vec3_rotate(cam->pos.v, -glm_rad(1.0f), cam->up.v);
	if (cam->dir & DIR_RIGHT)
		;// glm_vec3_rotate(cam->pos.v, glm_rad(1.0f), cam->up.v);
	if (cam->dir & DIR_UP || cam->dir & DIR_DOWN) {
		Vec3 pv = sub(cam->pos, VEC3(0.0f, 0.0f, 0.0f));
		// glm_vec3_cross(cam->up.v, pv.v, pv.v);
		if (cam->dir & DIR_DOWN)
			;// glm_vec3_rotate(cam->pos.v, -glm_rad(1.0f), pv.v);
		else
			;// glm_vec3_rotate(cam->pos.v, glm_rad(1.0f), pv.v);
	}

	cam->mats->view = lookat(cam->pos, VEC3(0.0f, 0.0f, 0.0f), cam->up);
	if (cam->type == CAMERA_ORTHOGONAL)
		camera_set_projection(cam, CAMERA_ORTHOGONAL);
}

struct Ray camera_get_mouse_ray(struct Camera* cam, float x, float y)
{
	Mat4x4 vp = multiply(cam->mats->proj, cam->mats->view);

	Vec3 p1, p2;
	// glm_unproject((vec3){ x, y, 0.0 }, vp, (float[]){ 0.0, 0.0, global_config.winw, global_config.winh }, p1);
	// glm_unproject((vec3){ x, y, 1.0 }, vp, (float[]){ 0.0, 0.0, global_config.winw, global_config.winh }, p2);

	return ray_from_points(p1, p2);
}

Vec2 camera_get_map_point(struct Ray ray)
{
	Vec3 p = ray_plane_intersection(ray, VEC4(0.0, 0.0, -1.0, 0.0));

	return (Vec2){ p.x, p.y };
}
