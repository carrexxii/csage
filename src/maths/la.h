#ifndef MATHS_LA_H
#define MATHS_LA_H

#include "types.h"
#include "macros.h"

#define VEC2(x, y)       (Vec2){ x, y       }
#define VEC3(x, y, z)    (Vec3){ x, y, z    }
#define VEC4(x, y, z, w) (Vec4){ x, y, z, w }
#define VEC3V(v) _Generic(v, Vec3: (Vec3){ v.x    , v.y    , v.z     }, \
                             Vec4: (Vec3){ v.x/v.w, v.y/v.w, v.z/v.w }, \
							 Vec : (Vec3){ v.x/v.w, v.y/v.w, v.z/v.w })
#define VEC4V(v) _Generic(v, Vec3: (Vec4){ v.arr[0], v.arr[1], v.arr[2]           }, \
                             Vec4: (Vec4){ v.arr[0], v.arr[1], v.arr[2], v.arr[3] }, \
							 Vec : (Vec4){ v.arr[0], v.arr[1], v.arr[2], v.arr[3] })

#define MAT4X4_IDENTITY (Mat4x4){ 1.0f, 0.0f, 0.0f, 0.0f, \
                                  0.0f, 1.0f, 0.0f, 0.0f, \
								  0.0f, 0.0f, 1.0f, 0.0f, \
								  0.0f, 0.0f, 0.0f, 1.0f, }

/* -------------------------------------------------------------------- */

#define is_zero(a, b) (fabs(a - b) <= FLT_EPSILON)
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

#endif
