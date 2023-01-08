#ifndef UTIL_MATHS_H
#define UTIL_MATHS_H

/* P(t) = p + t*v */
#include <math.h>
#include <vulkan/vulkan_core.h>
struct Ray {
	vec3 p;
	vec3 v;
}; static_assert(sizeof(struct Ray) == 24, "struct Ray");

struct Capsule {
	vec3 p1, p2;
	float r;
}; static_assert(sizeof(struct Capsule) == 28, "struct Capsule");

inline static void ivec3_copy_vec3(vec3 v, ivec3 u) {
	u[0] = (int)v[0];
	u[1] = (int)v[1];
	u[2] = (int)v[2];
}

inline static struct Ray ray_from_points(vec3 p1, vec3 p2) {
	struct Ray r;
	glm_vec3_copy(p1, r.p);
	glm_vec3_sub(p2, p1, r.v);
	glm_vec3_normalize(r.v);

	return r;
}

static void ray_print(struct Ray r) {
	fprintf(stderr, "Ray from (%.2f %.2f %.2f) in direction (%.2f %.2f %.2f)\n",
	        r.p[0], r.p[1], r.p[2], r.v[0], r.v[1], r.v[2]);
}

/* t = -(L (*) p)/(L (*) v) */
inline static void ray_plane_intersection(struct Ray r, vec4 L, vec3 out) {
	float t = -glm_vec3_dot(L, r.p) / glm_vec3_dot(L, r.v);
	glm_vec3_copy(r.p, out);
	glm_vec3_muladds(r.v, t, out);
}

inline static void ray_capsule_intersection(struct Ray ray, struct Capsule cap) {
	
}

/* Rect as [minx, miny, maxx, maxy] */
inline static bool point_in_rect(vec2 p, vec4 rect) {
	return ((p[0] >= rect[0] && p[0] <= rect[2]) &&
	        (p[1] >= rect[1] && p[1] <= rect[3]));
}

#endif

