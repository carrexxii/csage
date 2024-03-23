#ifndef CAMERA_H
#define CAMERA_H

#include "maths/types.h"
#include "gfx/buffers.h"

#define CAMERA_DEFAULT_PAN_SPEED  (15.0f*DT)
#define CAMERA_DEFAULT_ZOOM       2.0f
#define CAMERA_MIN_ZOOM           0.2f
#define CAMERA_MAX_ZOOM           10.0f
#define CAMERA_DEFAULT_ZOOM_SPEED (3.0f*DT)

enum CameraType {
	CAMERA_NONE,
	CAMERA_ORTHOGONAL,
	CAMERA_PERSPECTIVE,
};

struct Camera {
	UBO ubo;
	Vec3 pos;
	Vec3 up;
	Vec3 target;
	float pan_speed;
	float zoom;
	float zoom_speed;
	uint dir;
	enum CameraType type;
	float fov;
	float w, h;
	struct {
		Mat4x4 proj;
		Mat4x4 view;
	}* mats;
	Vec3* follow;
};

struct Camera camera_new(Vec3 pos, Vec3 up, float w, float h, float fov);
void camera_set_projection(struct Camera* cam, enum CameraType type);
void camera_move(struct Camera* cam, enum Direction dir, bool kdown);
void camera_rotate(struct Camera* cam, enum Axis axis, float angle);
void camera_update(struct Camera* cam);
struct Ray camera_get_mouse_ray(struct Camera* cam, float x, float y);
Vec2 camera_get_map_point(struct Ray ray);
void camera_free(struct Camera* cam);

#endif
