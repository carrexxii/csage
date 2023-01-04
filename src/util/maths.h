#ifndef UTIL_MATHS_H
#define UTIL_MATHS_H

struct Rect {
	float x, y, w, h;
}; static_assert(sizeof(struct Rect) == 16, "struct Rect");

inline static void mat4_set_pos(vec3 v, mat4 a) {
	a[3][0] = v[0];
	a[3][1] = v[1];
	a[3][2] = v[2];
}

inline static void ivec3_copy_vec3(vec3 v, ivec3 u)
{
	u[0] = (int)v[0];
	u[1] = (int)v[1];
	u[2] = (int)v[2];
}

inline static void ray_plane_intersection(vec3 p1, vec3 p2, vec4 plane, vec3 out) {
	vec3 ray;
	glm_vec3_sub(p2, p1, ray);
	float t = (plane[0]*p1[0] + plane[1]*p1[1] + plane[2]*p1[2] + plane[3])/ \
	          (plane[0]*(p1[0] - p2[0]) + plane[1]*(p1[1] - p2[1]) + plane[2]*(p1[2] - p2[2]));

	out[0] = p1[0] + t*(p2[0] - p1[0]);
	out[1] = p1[1] + t*(p2[1] - p1[1]);
	out[2] = p1[2] + t*(p2[2] - p1[2]);
}

#endif
