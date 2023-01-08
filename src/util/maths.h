#ifndef UTIL_MATHS_H
#define UTIL_MATHS_H

/* P(t) = p + t*v */
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

inline static bool vec3_is_zero(vec3 v) {
	return (fabs(v[0]) < GLM_FLT_EPSILON &&
	        fabs(v[1]) < GLM_FLT_EPSILON &&
	        fabs(v[2]) < GLM_FLT_EPSILON);
}

inline static void vec3_clamp(vec3 v, vec3 vmax, vec3 vmin) {
	CLAMP(v[0], vmin[0], vmax[0]);
	CLAMP(v[1], vmin[1], vmax[1]);
	CLAMP(v[2], vmin[2], vmax[2]);
}

/* This is modified from Paul Bourke's solution: http://paulbourke.net/geometry/pointlineplane/
 * Calculate the line segment p1p2 that is the shortest route between two lines p1p2 and p3p4.
 * Returns false if no solution exists.
 */
static bool line_line_nearest_points(vec3 p1, vec3 p2, vec3 p3, vec3 p4, vec3 pa, vec3 pb)
{
	vec3 p13, p43, p21;
	float d1343, d4321, d1321, d4343, d2121;
	float numer, denom;
	float s, t; /* s = mua; t = mub */

	glm_vec3_sub(p1, p3, p13);
	glm_vec3_sub(p4, p3, p43);
	if (vec3_is_zero(p43))
		return false;
	glm_vec3_sub(p2, p1, p21);
	if (vec3_is_zero(p21))
		return false;

	d1343 = glm_vec3_dot(p13, p43);
	d4321 = glm_vec3_dot(p43, p21);
	d1321 = glm_vec3_dot(p13, p21);
	d4343 = glm_vec3_dot(p43, p43);
	d2121 = glm_vec3_dot(p21, p21);

	denom = d2121*d4343 - d4321*d4321;
	if (fabs(denom) < GLM_FLT_EPSILON)
		return false;
	numer = d1343*d4321 - d1321*d4343;

	s = numer / denom;
	t = (d1343 + d4321*s) / d4343;

	pa[0] = p1[0] + s*p21[0];
	pa[1] = p1[1] + s*p21[1];
	pa[2] = p1[2] + s*p21[2];
	pb[0] = p3[0] + t*p43[0];
	pb[1] = p3[1] + t*p43[1];
	pb[2] = p3[2] + t*p43[2];

	return true;
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

/* out will be the point on the capsule's line segment */
inline static bool ray_capsule_intersection(struct Ray r, struct Capsule cap, vec3 out) {
	vec3 rp1, rp2, pa, pb;
	glm_vec3_copy(r.p, pa);
	glm_vec3_copy(r.p, pb);
	// glm_vec3_muladds(r.v,  100.0, pa);
	// glm_vec3_muladds(r.v, -100.0, pb);
	bool intersects = line_line_nearest_points(cap.p1, cap.p2, rp1, rp2, pa, pb);
	vec3_clamp(pa, cap.p1, cap.p2);
	glm_vec3_copy(pa, out);

	ray_print(r);
	DEBUG(1, "Capsule:");
	glm_vec3_print(cap.p1, stderr);
	glm_vec3_print(cap.p2, stderr);
	DEBUG_VALUE(cap.r);
	DEBUG(1, "\npa:");
	glm_vec3_print(pa, stderr);
	DEBUG(1, "pb:");
	glm_vec3_print(pb, stderr);
	DEBUG(1, "intersection: %s (dist: %f)", STRING_TF(intersects && (glm_vec3_distance2(pa, pb) <= cap.r*cap.r)),
	      glm_vec3_distance(pa, pb));
	DEBUG(1, " - - - -  - - - -  - - - -- - -- -- ");
	return intersects && (glm_vec3_distance2(pa, pb) <= cap.r*cap.r);
}

/* Rect as [minx, miny, maxx, maxy] */
inline static bool point_in_rect(vec2 p, vec4 rect) {
	return ((p[0] >= rect[0] && p[0] <= rect[2]) &&
	        (p[1] >= rect[1] && p[1] <= rect[3]));
}

#endif

