#ifndef MATHS_LA_H
#define MATHS_LA_H

#include "types.h"
#include "macros.h"

/* -------------------------------------------------------------------- */

#define IZ(a, b) (fabsf(a - b) <= FLT_EPSILON)
static inline bool vec2_equal_vec2(Vec2 v, Vec2 u) { return IZ(v.x, u.x) && IZ(v.y, u.y);                                 }
static inline bool vec3_equal_vec3(Vec3 v, Vec3 u) { return IZ(v.x, u.x) && IZ(v.y, u.y) && IZ(v.z, u.z);                 }
static inline bool vec4_equal_vec4(Vec4 v, Vec4 u) { return IZ(v.x, u.x) && IZ(v.y, u.y) && IZ(v.z, u.z) && IZ(v.w, u.w); }
#undef IZ
static inline bool vec2i_equal_vec2i(Vec2i v, Vec2i u) { return !(v.x - u.x) && !(v.y - u.y);                                 }
static inline bool vec3i_equal_vec3i(Vec3i v, Vec3i u) { return !(v.x - u.x) && !(v.y - u.y) && !(v.z - u.z);                 }
static inline bool vec4i_equal_vec4i(Vec4i v, Vec4i u) { return !(v.x - u.x) && !(v.y - u.y) && !(v.z - u.z) && !(v.w - u.w); }

static inline Vec2 vec2_add_scalar(Vec2 v, float s) { return (Vec2){ v.x + s, v.y + s                   }; }
static inline Vec3 vec3_add_scalar(Vec3 v, float s) { return (Vec3){ v.x + s, v.y + s, v.z + s          }; }
static inline Vec4 vec4_add_scalar(Vec4 v, float s) { return (Vec4){ v.x + s, v.y + s, v.z + s, v.w + s }; }
static inline Vec2 scalar_add_vec2(float s, Vec2 v) { return vec2_add_scalar(v, s); }
static inline Vec3 scalar_add_vec3(float s, Vec3 v) { return vec3_add_scalar(v, s); }
static inline Vec4 scalar_add_vec4(float s, Vec4 v) { return vec4_add_scalar(v, s); }
static inline Vec2i vec2i_add_scalar(Vec2i v, float s) { return (Vec2i){ v.x + s, v.y + s                   }; }
static inline Vec3i vec3i_add_scalar(Vec3i v, float s) { return (Vec3i){ v.x + s, v.y + s, v.z + s          }; }
static inline Vec4i vec4i_add_scalar(Vec4i v, float s) { return (Vec4i){ v.x + s, v.y + s, v.z + s, v.w + s }; }
static inline Vec2i scalar_add_vec2i(float s, Vec2i v) { return vec2i_add_scalar(v, s); }
static inline Vec3i scalar_add_vec3i(float s, Vec3i v) { return vec3i_add_scalar(v, s); }
static inline Vec4i scalar_add_vec4i(float s, Vec4i v) { return vec4i_add_scalar(v, s); }

static inline Vec2 vec2_add_vec2(Vec2 v, Vec2 u) { return (Vec2){ v.x + u.x, v.y + u.y                       }; }
static inline Vec3 vec3_add_vec3(Vec3 v, Vec3 u) { return (Vec3){ v.x + u.x, v.y + u.y, v.z + u.z            }; }
static inline Vec4 vec4_add_vec4(Vec4 v, Vec4 u) { return (Vec4){ v.x + u.x, v.y + u.y, v.z + u.z, v.w + u.w }; }
static inline Vec2i vec2i_add_vec2i(Vec2i v, Vec2i u) { return (Vec2i){ v.x + u.x, v.y + u.y                       }; }
static inline Vec3i vec3i_add_vec3i(Vec3i v, Vec3i u) { return (Vec3i){ v.x + u.x, v.y + u.y, v.z + u.z            }; }
static inline Vec4i vec4i_add_vec4i(Vec4i v, Vec4i u) { return (Vec4i){ v.x + u.x, v.y + u.y, v.z + u.z, v.w + u.w }; }

static inline Vec2 vec2_sub_vec2(Vec2 v, Vec2 u) { return (Vec2){ v.x - u.x, v.y - u.y                       }; }
static inline Vec3 vec3_sub_vec3(Vec3 v, Vec3 u) { return (Vec3){ v.x - u.x, v.y - u.y, v.z - u.z            }; }
static inline Vec4 vec4_sub_vec4(Vec4 v, Vec4 u) { return (Vec4){ v.x - u.x, v.y - u.y, v.z - u.z, v.w - u.w }; }
static inline Vec2i vec2i_sub_vec2i(Vec2i v, Vec2i u) { return (Vec2i){ v.x - u.x, v.y - u.y                       }; }
static inline Vec3i vec3i_sub_vec3i(Vec3i v, Vec3i u) { return (Vec3i){ v.x - u.x, v.y - u.y, v.z - u.z            }; }
static inline Vec4i vec4i_sub_vec4i(Vec4i v, Vec4i u) { return (Vec4i){ v.x - u.x, v.y - u.y, v.z - u.z, v.w - u.w }; }

static inline Vec2 vec2_multiply_scalar(Vec2 v, float s) { return (Vec2){ v.x * s, v.y * s                   }; }
static inline Vec3 vec3_multiply_scalar(Vec3 v, float s) { return (Vec3){ v.x * s, v.y * s, v.z * s          }; }
static inline Vec4 vec4_multiply_scalar(Vec4 v, float s) { return (Vec4){ v.x * s, v.y * s, v.z * s, v.w * s }; }
static inline Vec2 scalar_multiply_vec2(float s, Vec2 v) { return vec2_multiply_scalar(v, s); }
static inline Vec3 scalar_multiply_vec3(float s, Vec3 v) { return vec3_multiply_scalar(v, s); }
static inline Vec4 scalar_multiply_vec4(float s, Vec4 v) { return vec4_multiply_scalar(v, s); }

static inline float vec2_dot_vec2(Vec2 v, Vec2 u) { return v.x*u.x + v.y*u.y;                     }
static inline float vec3_dot_vec3(Vec3 v, Vec3 u) { return v.x*u.x + v.y*u.y + v.z*u.z;           }
static inline float vec4_dot_vec4(Vec4 v, Vec4 u) { return v.x*u.x + v.y*u.y + v.z*u.z + v.w*u.w; }

static inline Vec2 vec2_normalized(Vec2 v) { float c = 1.0f / sqrtf(dot(v, v)); return (Vec2){ v.x*c, v.y*c               }; }
static inline Vec3 vec3_normalized(Vec3 v) { float c = 1.0f / sqrtf(dot(v, v)); return (Vec3){ v.x*c, v.y*c, v.z*c        }; }
static inline Vec4 vec4_normalized(Vec4 v) { float c = 1.0f / sqrtf(dot(v, v)); return (Vec4){ v.x*c, v.y*c, v.z*c, v.w*c }; }

static inline float vec2_distance2_vec2(Vec2 v, Vec2 u) {
	float dx = u.x - v.x;
	float dy = u.y - v.y;
	return dx*dx + dy*dy;
}
static inline float vec3_distance2_vec3(Vec3 v, Vec3 u) {
	float dx = u.x - v.x;
	float dy = u.y - v.y;
	float dz = u.z - v.z;
	return dx*dx + dy*dy + dz*dz;
}

static inline float vec2_distance_vec2(Vec2 v, Vec2 u) { return sqrtf(vec2_distance2_vec2(v, u)); }
static inline float vec3_distance_vec3(Vec3 v, Vec3 u) { return sqrtf(vec3_distance2_vec3(v, u)); }

static inline Vec3 cross(Vec3 v, Vec3 u) {
	return (Vec3){
		.x = v.y*u.z - v.z*u.y,
		.y = v.x*u.z - v.z*u.x,
		.z = v.x*u.y - v.y*u.x,
	};
}

#define IZ(a) (fabs(a) <= FLT_EPSILON)
static inline bool vec2_is_zero(Vec2 v) { return IZ(v.x) && IZ(v.y);                       }
static inline bool vec3_is_zero(Vec3 v) { return IZ(v.x) && IZ(v.y) && IZ(v.z);            }
static inline bool vec4_is_zero(Vec4 v) { return IZ(v.x) && IZ(v.y) && IZ(v.z) && IZ(v.w); }
#undef IZ

/* -------------------------------------------------------------------- */

static inline Mat4x4 mat4x4_multiply_scalar(Mat4x4 a, float s) {
	for (int i = 0; i < 16; i++)
		a.arr[i] *= s;
	return a;
}
static inline Mat4x4 scalar_multiply_mat4x4(float s, Mat4x4 a) { return mat4x4_multiply_scalar(a, s); }

static inline Mat4x4 mat4x4_multiply_mat4x4(Mat4x4 a, Mat4x4 b)
{
	return (Mat4x4){
		.m11 = a.m11*b.m11 + a.m12*b.m21 + a.m13*b.m31 + a.m14*b.m41,
		.m12 = a.m11*b.m12 + a.m12*b.m22 + a.m13*b.m32 + a.m14*b.m42,
		.m13 = a.m11*b.m13 + a.m12*b.m23 + a.m13*b.m33 + a.m14*b.m43,
		.m14 = a.m11*b.m14 + a.m12*b.m24 + a.m13*b.m34 + a.m14*b.m44,
		.m21 = a.m21*b.m11 + a.m22*b.m21 + a.m23*b.m31 + a.m24*b.m41,
		.m22 = a.m21*b.m12 + a.m22*b.m22 + a.m23*b.m32 + a.m24*b.m42,
		.m23 = a.m21*b.m13 + a.m22*b.m23 + a.m23*b.m33 + a.m24*b.m43,
		.m24 = a.m21*b.m14 + a.m22*b.m24 + a.m23*b.m34 + a.m24*b.m44,
		.m31 = a.m31*b.m11 + a.m32*b.m21 + a.m33*b.m31 + a.m34*b.m41,
		.m32 = a.m31*b.m12 + a.m32*b.m22 + a.m33*b.m32 + a.m34*b.m42,
		.m33 = a.m31*b.m13 + a.m32*b.m23 + a.m33*b.m33 + a.m34*b.m43,
		.m34 = a.m31*b.m14 + a.m32*b.m24 + a.m33*b.m34 + a.m34*b.m44,
		.m41 = a.m41*b.m11 + a.m42*b.m21 + a.m43*b.m31 + a.m44*b.m41,
		.m42 = a.m41*b.m12 + a.m42*b.m22 + a.m43*b.m32 + a.m44*b.m42,
		.m43 = a.m41*b.m13 + a.m42*b.m23 + a.m43*b.m33 + a.m44*b.m43,
		.m44 = a.m41*b.m14 + a.m42*b.m24 + a.m43*b.m34 + a.m44*b.m44,
	};
}

static inline Vec4 mat4x4_multiply_vec4(Mat4x4 a, Vec4 v)
{
	return (Vec4){
		a.m11*v.x + a.m21*v.y + a.m31*v.z + a.m41*v.w,
		a.m12*v.x + a.m22*v.y + a.m32*v.z + a.m42*v.w,
		a.m13*v.x + a.m23*v.y + a.m33*v.z + a.m43*v.w,
		a.m14*v.x + a.m24*v.y + a.m34*v.z + a.m44*v.w,
	};
}
static inline Vec4 vec4_multiply_mat4x4(Vec4 v, Mat4x4 a) { return mat4x4_multiply_vec4(a, v); }

static inline Vec3 vec3_lerp(Vec3 v, Vec3 u, float f)
{
	Vec3 diff = vec3_sub_vec3(u, v);
	diff = vec3_multiply_scalar(diff, f);
	return vec3_add_vec3(v, diff);
}

/* -------------------------------------------------------------------- */

static Mat4x4 lookat(Vec3 eye, Vec3 center, Vec3 up)
{
	Vec3 f = vec3_normalized(sub(center, eye));
	Vec3 s = vec3_normalized(cross(f, up));
	Vec3 u = cross(s, f);
	return (Mat4x4){
		.m11 = s.x, .m12 = u.x, .m13 = -f.x,
		.m21 = s.y, .m22 = u.y, .m23 = -f.y,
		.m31 = s.z, .m32 = u.z, .m33 = -f.z,
		.m41 = -dot(s, eye),
		.m42 = -dot(u, eye),
		.m43 =  dot(f, eye),
		.m44 = 1.0f,
	};
}

static Mat4x4 cam_orthogonal(float left, float right, float bot, float top, float near, float far)
{
	float x = 2.0f / (right - left);
	float y = 2.0f / (bot - top);
	float z = 1.0f / (near - far);
	float px = -(right + left) / (right - left);
	float py = -(bot   + top)  / (bot   - top);
	float pz = near / (near - far);
	return (Mat4x4){
		x   , 0.0f, 0.0f, 0.0f,
		0.0f, y   , 0.0f, 0.0f,
		0.0f, 0.0f, z   , 0.0f,
		px  , py  , pz  , 1.0f,
	};
}

static Mat4x4 cam_perspective(float ar, float fov, float near, float far)
{
	float f  = 1.0f / tanf(0.5f*fov);
	float z  = far / (near - far);
	float nf = (near*far) / (near - far);
	return (Mat4x4){
		f/ar, 0.0f, 0.0f,  0.0f,
		0.0f, -f  , 0.0f,  0.0f,
		0.0f, 0.0f, z   , -1.0f,
		0.0f, 0.0f, nf  ,  0.0f,
	};
}

static inline void translate(Mat4x4* a, Vec3 v)
{
	a->m41 += v.x;
	a->m42 += v.y;
	a->m43 += v.z;
}

static inline void translate_to(Mat4x4* a, Vec3 v)
{
	a->m41 = v.x;
	a->m42 = v.y;
	a->m43 = v.z;
}

static inline Mat4x4 translate_make(Vec3 v)
{
	return (Mat4x4){
		.m41 = v.x,
		.m42 = v.y,
		.m43 = v.z,
		.m11 = 1.0f,
		.m22 = 1.0f,
		.m33 = 1.0f,
		.m44 = 1.0f,
	};
}

// https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle:~:text=Rotation%20matrix%20from%20axis%20and%20angle%5Bedit%5D
/* Axis should be normalized */
static Mat4x4 rotate_make(Vec3 v, float angle)
{
	float s = sinf(angle);
	float c = cosf(angle);
	return (Mat4x4){
		c + v.x*v.x*(1.0f - c)    , v.x*v.y*(1.0f - c) - v.z*s, v.x*v.z*(1.0f - c) + v.y*s, 0.0f,
		v.y*v.x*(1.0f - c) + v.z*s, c + v.y*v.y*(1.0f - c)    , v.y*v.z*(1.0f - c) - v.x*s, 0.0f,
		v.z*v.x*(1.0f - c) - v.y*s, v.z*v.y*(1.0f - c) + v.x*s, c + v.z*v.z*(1.0f - c)    , 0.0f,
		0.0f                      ,                       0.0f,                       0.0f, 1.0f,
	};
}

/* Rodrigues' rotation formula:
 *   v = vcos(t) + (k×v)sin(t) + k(k⋅v)(1 - cos(t))
 */
static inline Vec3 rotate_vec3(Vec3 v, Vec3 axis, float angle)
{
	Vec3 k = vec3_normalized(axis);
	float cost = cosf(angle);
	float sint = sinf(angle);

	Vec3 res = vec3_multiply_scalar(v, cost);
	     res = vec3_add_vec3(res, vec3_multiply_scalar(cross(k, v), sint));
	     res = vec3_add_vec3(res, vec3_multiply_scalar(vec3_multiply_scalar(k, dot(k, v)), 1.0f - cost));
	return res;
}

static inline Mat4x4 rotate_mat4x4(Mat4x4 a, Vec3 axis, float angle) {
	return mat4x4_multiply_mat4x4(a, rotate_make(axis, angle));
}

static void print_mat4x4(Mat4x4 a)
{
	fprintf(stderr, "Mat4x4: %5.2f, %5.2f, %5.2f, %5.2f\n", a.m11, a.m12, a.m13, a.m14);
	fprintf(stderr, "        %5.2f, %5.2f, %5.2f, %5.2f\n", a.m21, a.m22, a.m23, a.m24);
	fprintf(stderr, "        %5.2f, %5.2f, %5.2f, %5.2f\n", a.m31, a.m32, a.m33, a.m34);
	fprintf(stderr, "        %5.2f, %5.2f, %5.2f, %5.2f\n", a.m41, a.m42, a.m43, a.m44);
}

/* https://stackoverflow.com/a/44446912 */
static Mat4x4 mat4x4_inverse(Mat4x4 a)
{
	float a2323 = a.m33*a.m44 - a.m34*a.m43;
	float a1323 = a.m32*a.m44 - a.m34*a.m42;
	float a1223 = a.m32*a.m43 - a.m33*a.m42;
	float a0323 = a.m31*a.m44 - a.m34*a.m41;
	float a0223 = a.m31*a.m43 - a.m33*a.m41;
	float a0123 = a.m31*a.m42 - a.m32*a.m41;
	float a2313 = a.m23*a.m44 - a.m24*a.m43;
	float a1313 = a.m22*a.m44 - a.m24*a.m42;
	float a1213 = a.m22*a.m43 - a.m23*a.m42;
	float a2312 = a.m23*a.m34 - a.m24*a.m33;
	float a1312 = a.m22*a.m34 - a.m24*a.m32;
	float a1212 = a.m22*a.m33 - a.m23*a.m32;
	float a0313 = a.m21*a.m44 - a.m24*a.m41;
	float a0213 = a.m21*a.m43 - a.m23*a.m41;
	float a0312 = a.m21*a.m34 - a.m24*a.m31;
	float a0212 = a.m21*a.m33 - a.m23*a.m31;
	float a0113 = a.m21*a.m42 - a.m22*a.m41;
	float a0112 = a.m21*a.m32 - a.m22*a.m31;

	float det = a.m11 * (a.m22*a2323 - a.m23*a1323 + a.m24*a1223) -
	            a.m12 * (a.m21*a2323 - a.m23*a0323 + a.m24*a0223) +
	            a.m13 * (a.m21*a1323 - a.m22*a0323 + a.m24*a0123) -
	            a.m14 * (a.m21*a1223 - a.m22*a0223 + a.m23*a0123);
	det = 1.0f / det;

	return (Mat4x4){
		.m11 = det *  (a.m22*a2323 - a.m23*a1323 + a.m24*a1223),
		.m12 = det * -(a.m12*a2323 - a.m13*a1323 + a.m14*a1223),
		.m13 = det *  (a.m12*a2313 - a.m13*a1313 + a.m14*a1213),
		.m14 = det * -(a.m12*a2312 - a.m13*a1312 + a.m14*a1212),
		.m21 = det * -(a.m21*a2323 - a.m23*a0323 + a.m24*a0223),
		.m22 = det *  (a.m11*a2323 - a.m13*a0323 + a.m14*a0223),
		.m23 = det * -(a.m11*a2313 - a.m13*a0313 + a.m14*a0213),
		.m24 = det *  (a.m11*a2312 - a.m13*a0312 + a.m14*a0212),
		.m31 = det *  (a.m21*a1323 - a.m22*a0323 + a.m24*a0123),
		.m32 = det * -(a.m11*a1323 - a.m12*a0323 + a.m14*a0123),
		.m33 = det *  (a.m11*a1313 - a.m12*a0313 + a.m14*a0113),
		.m34 = det * -(a.m11*a1312 - a.m12*a0312 + a.m14*a0112),
		.m41 = det * -(a.m21*a1223 - a.m22*a0223 + a.m23*a0123),
		.m42 = det *  (a.m11*a1223 - a.m12*a0223 + a.m13*a0123),
		.m43 = det * -(a.m11*a1213 - a.m12*a0213 + a.m13*a0113),
		.m44 = det *  (a.m11*a1212 - a.m12*a0212 + a.m13*a0112),
	};
}

static inline Vec3 unproject(Vec3 v, Mat4x4 a, Rect vp)
{
	Mat4x4 a_inv = mat4x4_inverse(a);
	Vec4 u = VEC4(2.0f*(v.x - vp.x) / vp.w - 1.0f,
	              2.0f*(v.y - vp.y) / vp.h - 1.0f,
				  v.z,
				  1.0f);
	u = mat4x4_multiply_vec4(a_inv, u);
	u = vec4_multiply_scalar(u, 1.0f / u.w);

	return VEC3_V4(u);
}

#endif
