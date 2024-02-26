#ifndef MATHS_LA_H
#define MATHS_LA_H

#include "types.h"
#include "macros.h"

#define VEC2(x, y)       (Vec2){ x, y       }
#define VEC3(x, y, z)    (Vec3){ x, y, z    }
#define VEC4(x, y, z, w) (Vec4){ x, y, z, w }
#define VEC2I(x, y)       (Vec2i){ x, y       }
#define VEC3I(x, y, z)    (Vec3i){ x, y, z    }
#define VEC4I(x, y, z, w) (Vec4i){ x, y, z, w }
#define VEC3_V3(v)  (Vec3){ v.x    , v.y    , v.z     }
#define VEC3_V4(v)  (Vec3){ v.x/v.w, v.y/v.w, v.z/v.w }
#define VEC3_A(arr) (Vec3){ arr[0] , arr[1] , arr[2]  }
#define VEC3I_V(v)   (Vec3i){ v.x    , v.y    , v.z     }
#define VEC3I_A(arr) (Vec3i){ arr[0] , arr[1] , arr[2]  }
#define VEC4_V3(v)  (Vec4){ v.x   , v.y   , v.z   , 0.0f    }
#define VEC4_V4(v)  (Vec4){ v.x   , v.y   , v.z   , v.w     }
#define VEC4_A(arr) (Vec4){ arr[0], arr[1], arr[2], arr[3]  }

#define VEC3_ZERO (Vec3){ 0 }
#define MAT4X4_IDENTITY (Mat4x4){ 1.0f, 0.0f, 0.0f, 0.0f, \
                                  0.0f, 1.0f, 0.0f, 0.0f, \
								  0.0f, 0.0f, 1.0f, 0.0f, \
								  0.0f, 0.0f, 0.0f, 1.0f, }

/* -------------------------------------------------------------------- */

#define is_zero(a, b) (fabsf(a - b) <= FLT_EPSILON)
static inline bool vec2_equal_vec2(Vec2 v, Vec2 u) { return is_zero(v.x, u.x) && is_zero(v.y, u.y);                                           }
static inline bool vec3_equal_vec3(Vec3 v, Vec3 u) { return is_zero(v.x, u.x) && is_zero(v.y, u.y) && is_zero(v.z, u.z);                      }
static inline bool vec4_equal_vec4(Vec4 v, Vec4 u) { return is_zero(v.x, u.x) && is_zero(v.y, u.y) && is_zero(v.z, u.z) && is_zero(v.w, u.w); }
#undef is_zero
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

/* -------------------------------------------------------------------- */

static inline Mat4x4 mat4x4_multiply_scalar(Mat4x4 a, float s) {
	for (int i = 0; i < 16; i++)
		a.arr[i] *= s;
	return a;
}
static inline Mat4x4 scalar_multiply_mat4x4(float s, Mat4x4 a) { return mat4x4_multiply_scalar(a, s); }

static inline Mat4x4 mat4x4_multiply_mat4x4(Mat4x4 a, Mat4x4 b) {
	return (Mat4x4){
		.m11 = a.m11*b.m11 + a.m11*b.m21 + a.m12*b.m31 + a.m14*b.m41,
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

/* -------------------------------------------------------------------- */

static inline Mat4x4 lookat(Vec3 eye, Vec3 center, Vec3 up) {
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

#endif
