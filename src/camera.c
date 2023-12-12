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
	if (kdown)
		cam->dir |= dir;
	else
		cam->dir &= ~dir;
}

void camera_update(struct Camera* cam)
{
	Vec3 dir = { 0 };
	if (cam->dir & DIR_FORWARDS || cam->dir & DIR_BACKWARDS) {
		glm_vec3_sub(cam->pos.v, (vec3){ 0.0f, 0.0f, 0.0f }, dir.v);
		glm_vec3_normalize(dir.v);
		if (cam->dir & DIR_FORWARDS)
			glm_vec3_scale(dir.v, -0.1f, dir.v);
		else
			glm_vec3_scale(dir.v, 0.1f, dir.v);
	}
	glm_vec3_add(cam->pos.v, dir.v, cam->pos.v);

	if (cam->dir & DIR_LEFT)
		glm_vec3_rotate(cam->pos.v, -glm_rad(1.0f), cam->up.v);
	if (cam->dir & DIR_RIGHT)
		glm_vec3_rotate(cam->pos.v, glm_rad(1.0f), cam->up.v);
	if (cam->dir & DIR_UP || cam->dir & DIR_DOWN) {
		Vec3 pv;
		glm_vec3_sub(cam->pos.v, (vec3){ 0.0f, 0.0f, 0.0f }, pv.v);
		glm_vec3_cross(cam->up.v, pv.v, pv.v);
		if (cam->dir & DIR_DOWN)
			glm_vec3_rotate(cam->pos.v, -glm_rad(1.0f), pv.v);
		else
			glm_vec3_rotate(cam->pos.v, glm_rad(1.0f), pv.v);
	}

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
