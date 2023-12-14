#ifndef CAMERA_H
#define CAMERA_H

#include "config.h"
#include "maths/maths.h"

#define CAMERA_MIN_ZOOM 0.0
#define CAMERA_MAX_ZOOM 500.0

enum CameraType {
	CAMERA_NONE,
	CAMERA_ORTHOGONAL,
	CAMERA_PERSPECTIVE,
};

struct Camera {
	Vec3 pos;
	Vec3 up;
	uint dir;
	enum CameraType type;
	float fov;
	float w, h;
	struct {
		Mat4x4 proj;
		Mat4x4 view;
	}* mats;
};

struct Camera camera_new(Vec3 pos, Vec3 up, float w, float h, float fov);
void camera_set_projection(struct Camera* cam, enum CameraType type);
void camera_move(struct Camera* cam, enum Direction dir, bool kdown);
void camera_update(struct Camera* cam);
struct Ray camera_get_mouse_ray(struct Camera* cam, float x, float y);
vec2s camera_get_map_point(struct Ray ray);

#endif
