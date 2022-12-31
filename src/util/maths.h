#ifndef UTIL_MATHS_H
#define UTIL_MATHS_H

struct Rect {
	float x, y, w, h;
}; static_assert(sizeof(struct Rect) == 16, "struct Rect");

inline static void mat4_set_pos(vec3 v, mat4 a) {
	a[0][3] = v[0];
	a[1][3] = v[1];
	a[2][3] = v[2];
}

#endif
