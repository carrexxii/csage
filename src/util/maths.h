#ifndef UTIL_MATHS_H
#define UTIL_MATHS_H

typedef uint uvec3[3];

struct Rect {
	float x, y, w, h;
}; static_assert(sizeof(struct Rect) == 16, "struct Rect");

/* dst last is cglm convention */
static inline void uvec3_copy(uvec3 src, uvec3 dst) {
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

inline static void mat4_set_pos(vec3 v, mat4 a) {
	a[0][3] = v[0];
	a[1][3] = v[1];
	a[2][3] = v[2];
}

#endif
