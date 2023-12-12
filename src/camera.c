#include "config.h"
#include "maths/maths.h"
#include "input.h"
#include "camera.h"

struct Camera camera_new(Vec3 pos, Vec3 up)
{
	struct Camera cam = {
		.pos  = pos,
		.up   = up,
		.mats = smalloc(sizeof(Mat4x4[2])),
	};

	glm_mat4_identity(cam.mats->proj.m);
	glm_translate_make(cam.mats->view.m, pos.v);
	camera_update(&cam);

	return cam;
}

void camera_set_ortho(struct Camera* cam, Rect r)
{
	glm_ortho(r.x - r.w/2.0f, r.x + r.w/2.0f,
	          r.y - r.h/2.0f, r.y + r.h/2.0f,
	          -1.0f, 1024.0f, cam->mats->proj.m);
}

void camera_set_persp(struct Camera* cam, float w, float h, float fov)
{
	glm_perspective(fov, w / h, 0.1f, 1024.0f, cam->mats->proj.m);
}

void camera_move(struct Camera* cam, enum Direction dir, bool kdown)
{
	switch (dir) {
	case DIR_UP:
	case DIR_DOWN:
	case DIR_LEFT:
	case DIR_RIGHT:
	case DIR_FORWARDS:
	case DIR_BACKWARDS:
		if (kdown)
			cam->dir |= dir;
		else
			cam->dir &= ~dir;
		break;
	default:
		ERROR("[CAM] Invalid direction: %d", dir);
	}
}

void camera_update(struct Camera* cam)
{
	Vec3 vel = {
		cam->dir & DIR_LEFT? 0.1f: cam->dir & DIR_RIGHT? -0.1f: 0.0f,
		0.0f,
		cam->dir & (DIR_FORWARDS | DIR_UP)? 0.1f: cam->dir & (DIR_BACKWARDS | DIR_DOWN)? -0.1f: 0.0f,
	};
	glm_vec3_add(cam->pos.v, vel.v, cam->pos.v);
	glm_lookat(cam->pos.v, (vec3){ 0.0f, 0.0f, 0.0f }, cam->up.v, cam->mats->view.m);
}

struct Ray camera_get_mouse_ray(struct Camera* cam, float x, float y)
{
	mat4 vp;
	glm_mat4_mul(cam->mats->proj.m, cam->mats->view.m, vp);

	vec3 p1, p2;
	glm_unproject((vec3){ x, y, 0.0 }, vp, (float[]){ 0.0, 0.0, global_config.winw, global_config.winh }, p1);
	glm_unproject((vec3){ x, y, 1.0 }, vp, (float[]){ 0.0, 0.0, global_config.winw, global_config.winh }, p2);

	return ray_from_points(p1, p2);
}

vec2s camera_get_map_point(struct Ray ray)
{
	vec3 p;
	ray_plane_intersection(ray, (vec4){ 0.0, 0.0, -1.0, 0.0 }, p);

	return (vec2s){ p[0], p[1] };
}
