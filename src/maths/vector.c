#include "maths.h"

#if DEBUG_LEVEL > 0
	void vec2_print(vec2 v)
	{
		double m = (double)vec_mag(v);
		DEBUG(1, "[Vec2(%.3f): %+-.3f|%+-.3f]", m, v.x, v.y);
	}
	void vec2_printp(vec2* v)  { vec2_print(*v); }

	void vec3_print(vec3 v)
	{
		double m = (double)vec_mag(v);
		DEBUG(1, "[Vec3(%.3f): %+-.3f|%+-.3f|%+-.3f]", m, v.x, v.y, v.z);
	}
	void vec3_printp(vec3* v)  { vec3_print(*v); }

	void vec4_print(vec4 v)
	{
		double m = (double)vec_mag(v);
		DEBUG(1, "[Vec4(%.3f): %+-.3f|%+-.3f|%+-.3f|%+-.3f]", m, v.x, v.y, v.z, v.w);
	}
	void vec4_printp(vec4* v)  { vec4_print(*v); }
#else
	#define vec2_print(v)
	#define vec3_print(v)
	#define vec4_print(v)
#endif

vec3 vec_from_dir(enum Direction dir)
{
	vec3 v = VEC3(0, 0, 0);
	switch (dir) {
		case DIR_NONE     : break;
		case DIR_RIGHT    : v.x =  1.0f; break;
		case DIR_LEFT     : v.x = -1.0f; break;
		case DIR_UP       : v.y =  1.0f; break;
		case DIR_DOWN     : v.y = -1.0f; break;
		case DIR_BACKWARDS: v.z = -1.0f; break;
		case DIR_FORWARDS : v.z =  1.0f; break;
		default:
			ERROR("%d is not a valid direction", dir);
	}

	return v;
}
