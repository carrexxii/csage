#ifndef UTIL_MATHS_H
#define UTIL_MATHS_H

#include <math.h>
#include <float.h>

#include "types.h"
#include "la.h"
#include "pga.h"
#include "macros.h"

#define E       2.7182818284590452354
#define PI      3.14159265358979323846
#define PI_2    1.57079632679489661923
#define PI_4    0.78539816339744830962
#define SQRT2   1.41421356237309504880
#define SQRT1_2 0.70710678118654752440

#define  RECT(x, y, w, h)  (Rect){ x, y, w, h }
#define RECTI(x, y, w, h) (Recti){ x, y, w, h }

#define print_rect(r) _Generic((r), \
	Rect : print_rect_rect,         \
	Recti: print_rect_recti)(r)
static inline void print_rect_rect(Rect rect) {
	printf("Rect -> [%.2f, %.2f, %.2f, %.2f]\n", rect.x, rect.y, rect.w, rect.h);
}
static inline void print_rect_recti(Recti rect) {
	printf("Recti -> [%d, %d, %d, %d]\n", rect.x, rect.y, rect.w, rect.h);
}

static inline float deg_to_rad(float d) { return d/360.0f * 2.0f*PI; }
static inline float rad_to_deg(float r) { return r/2.0f*PI * 360.0f; }

// static inline ivec3s ivec3s_of_vec3s(vec3s v)  { return (ivec3s){ .x = (int)v.x, .y = (int)v.y, .z = (int)v.z }; }
// static inline vec3s  vec3s_of_ivec3s(ivec3s v) { return (vec3s){ .x = (float)v.x, .y = (float)v.y, .z = (float)v.z }; }
// static inline vec3s  vec3s_of_int8(int8 v[3])  { return (vec3s){ .x = (float)v[0], .y = (float)v[1], .z = (float)v[2] }; }

// static inline void ivec3_copy_vec3(vec3 v, ivec3 u) {
// 	u[0] = (int)v[0];
// 	u[1] = (int)v[1];
// 	u[2] = (int)v[2];
// }

// static inline bool vec3_is_zero(vec3 v) {
// 	return (fabs(v[0]) < GLM_FLT_EPSILON &&
// 	        fabs(v[1]) < GLM_FLT_EPSILON &&
// 	        fabs(v[2]) < GLM_FLT_EPSILON);
// }

// static inline void vec3_clamp(vec3 v, vec3 vmax, vec3 vmin) {
// 	CLAMP(v[0], vmin[0], vmax[0]);
// 	CLAMP(v[1], vmin[1], vmax[1]);
// 	CLAMP(v[2], vmin[2], vmax[2]);
// }

// static inline bool ivec3s_eq(ivec3s v, ivec3s u) {
// 	return v.x == u.x && v.y == u.y && v.z == u.z;
// }

// static inline ivec3s ivec3s_add(ivec3s v, ivec3s u) {
// 	return (ivec3s){
// 		.x = v.x + u.x,
// 		.y = v.y + u.y,
// 		.z = v.z + u.z,
// 	};
// }

/* This is modified from Paul Bourke's solution: http://paulbourke.net/geometry/pointlineplane/
 * Calculate the line segment p1p2 that is the shortest route between two lines p1p2 and p3p4.
 * Returns false if no solution exists.
 */
static bool line_line_nearest_points(Vec3 p1, Vec3 p2, Vec3 p3, Vec3 p4, Vec3* pa, Vec3* pb)
{
	Vec3 p13 = sub(p1, p3);
	Vec3 p43 = sub(p4, p3);
	if (vec3_is_zero(p43))
		return false;
	Vec3 p21 = sub(p2, p1);
	if (vec3_is_zero(p21))
		return false;

	float d1343 = dot(p13, p43);
	float d4321 = dot(p43, p21);
	float d1321 = dot(p13, p21);
	float d4343 = dot(p43, p43);
	float d2121 = dot(p21, p21);

	float denom = d2121*d4343 - d4321*d4321;
	if (fabs(denom) < FLT_EPSILON)
		return false;
	float numer = d1343*d4321 - d1321*d4343;

	float s = numer / denom;
	float t = (d1343 + d4321*s) / d4343;

	pa->x = p1.x + s*p21.x;
	pa->y = p1.y + s*p21.y;
	pa->z = p1.z + s*p21.z;
	pb->x = p3.x + t*p43.x;
	pb->y = p3.y + t*p43.y;
	pb->z = p3.z + t*p43.z;

	return true;
}

static inline struct Ray ray_from_points(Vec3 p1, Vec3 p2) {
	Vec3 d = sub(p2, p1);
	struct Ray r = {
		.p = p1,
		.v = normalized(d),
	};

	return r;
}

static void ray_print(struct Ray r) {
	fprintf(stderr, "Ray from (%.2f %.2f %.2f) in dRectiion (%.2f %.2f %.2f)\n",
	        r.p.x, r.p.y, r.p.z, r.v.x, r.v.y, r.v.z);
}

/* t = -(L (*) p)/(L (*) v) */
static inline Vec3 ray_plane_intersection(struct Ray r, Vec4 L) {
	float t = -dot(VEC3_V4(L), r.p) / dot(VEC3_V4(L), r.v);
	return add(r.p, multiply(r.v, t));
}

static inline bool point_in_rect(Vec2 p, Rect rect) {
	return ((p.x >= rect.x && p.x <= rect.w) &&
	        (p.y >= rect.y && p.y <= rect.h));
}

static inline float polygon_moment(int vertc, Vec2 verts, Vec2 cm, float m)
{
	float I = 0;
	float r;
	float* v;
	for (int i = 0; i < vertc/2; i++) {
		v = (float*)verts.arr + i;
		r = distance(cm, VEC2(v[0], v[1]));
		I += r*r;
	}

	return I*m/12.0;
}

#endif
