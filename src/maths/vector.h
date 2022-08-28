#ifndef MATHS_VECTOR_H
#define MATHS_VECTOR_H

typedef union {
	float arr[2];
	struct { float x, y; };
	struct { float i, j; };
	struct { float u, v; };
	struct { float w, h; };
} vec2; static_assert(sizeof(vec2) == 8, "vec2");

typedef union {
	float arr[3];
	struct { float x, y, z; };
	struct { float i, j, k; };
	struct { float r, g, b; };
	struct { float w, h, d; };
} vec3; static_assert(sizeof(vec3) == 12, "vec3");

typedef union {
	float arr[4];
	struct { float x, y, z, w; };
	struct { float i, j, k, l; };
	struct { float r, g, b, a; };
} vec4; static_assert(sizeof(vec4) == 16, "vec4");

typedef union {
    uint32 arr[3];
    struct { uint32 x, y, z; };
    struct { uint32 i, j, k; };
    struct { uint32 w, h, d; };
} uvec3; static_assert(sizeof(uvec3) == 12, "uvec3");

#define VEC2(x, y)       (vec2){ (x), (y)           }
#define VEC3(x, y, z)    (vec3){ (x), (y), (z)      }
#define VEC4(x, y, z, w) (vec4){ (x), (y), (z), (w) }
#define VEC2A(v)         (vec2){ (v)[0], (v)[1] s                }
#define VEC3A(v)         (vec3){ (v)[0], (v)[1], (v)[2]         }
#define VEC4A(v)         (vec4){ (v)[0], (v)[1], (v)[2], (v)[3] }
#define VEC2V(v)         (vec2){ (v).x, (v).y               }
#define VEC3V(v)         (vec3){ (v).x, (v).y, (v).z        }
#define VEC4V(v)         (vec4){ (v).x, (v).y, (v).z, (v).w }
#define VEC2P(v)         (vec2){ (v)->x, (v)->y                 }
#define VEC3P(v)         (vec3){ (v)->x, (v)->y, (v)->z         }
#define VEC4P(v)         (vec4){ (v)->x, (v)->y, (v)->z, (v)->w }
#define VEC2_ZERO        (vec2){ 0.0, 0.0           }
#define VEC3_ZERO        (vec3){ 0.0, 0.0, 0.0      }
#define VEC4_ZERO        (vec4){ 0.0, 0.0, 0.0, 0.0 }

#define UVEC3(x, y, z) (uvec3){ (x), (y), (z), }

/*****************************************************************************/

#define vec_scale_ip(v, s) _Generic((v), vec2*: vec2_scale_ip, \
                                         vec3*: vec3_scale_ip, \
                                         vec4*: vec4_scale_ip)(v, s)
inline static void vec2_scale_ip(vec2* v, float s) { cblas_sscal(2, s, v->arr, 1); }
inline static void vec3_scale_ip(vec3* v, float s) { cblas_sscal(3, s, v->arr, 1); }
inline static void vec4_scale_ip(vec4* v, float s) { cblas_sscal(4, s, v->arr, 1); }

#define vec_scale(v, s) _Generic((v), vec2: vec2_scale, \
                                      vec3: vec3_scale, \
                                      vec4: vec4_scale)(v, s)
inline static vec2 vec2_scale(vec2 v, float s) { vec2 u = VEC2V(v); vec_scale_ip(&u, s); return u; }
inline static vec3 vec3_scale(vec3 v, float s) { vec3 u = VEC3V(v); vec_scale_ip(&u, s); return u; }
inline static vec4 vec4_scale(vec4 v, float s) { vec4 u = VEC4V(v); vec_scale_ip(&u, s); return u; }

#define vec_mag(v) _Generic((v), vec2: vec2_mag, \
                                 vec3: vec3_mag, \
                                 vec4: vec4_mag)(v)
inline static float vec2_mag(vec2 v) { return cblas_snrm2(2, v.arr, 1); }
inline static float vec3_mag(vec3 v) { return cblas_snrm2(3, v.arr, 1); }
inline static float vec4_mag(vec4 v) { return cblas_snrm2(4, v.arr, 1); }

#define vec_copy(v, u) _Generic((v), vec2*: vec2_copy, \
                                     vec3*: vec3_copy, \
                                     vec4*: vec4_copy)(v, u)
inline static void vec2_copy(vec2* v, vec2 u) { cblas_scopy(2, u.arr, 1, v->arr, 1); }
inline static void vec3_copy(vec3* v, vec3 u) { cblas_scopy(3, u.arr, 1, v->arr, 1); }
inline static void vec4_copy(vec4* v, vec4 u) { cblas_scopy(4, u.arr, 1, v->arr, 1); }

#define vec_add_ip(v, u) _Generic((v), vec2*: vec2_add_ip, \
                                       vec3*: vec3_add_ip, \
                                       vec4*: vec4_add_ip)(v, u)
inline static void vec2_add_ip(vec2* v, vec2 u) { cblas_saxpy(2, 1, u.arr, 1, v->arr, 1); }
inline static void vec3_add_ip(vec3* v, vec3 u) { cblas_saxpy(3, 1, u.arr, 1, v->arr, 1); }
inline static void vec4_add_ip(vec4* v, vec4 u) { cblas_saxpy(4, 1, u.arr, 1, v->arr, 1); }

#define vec_add_scale_ip(v, u, s) _Generic((v), vec2*: vec2_add_scale_ip, \
                                                vec3*: vec3_add_scale_ip, \
                                                vec4*: vec4_add_scale_ip)(v, u, s)
inline static void vec2_add_scale_ip(vec2* v, vec2 u, float s) { cblas_saxpy(2, s, u.arr, 1, v->arr, 1); }
inline static void vec3_add_scale_ip(vec3* v, vec3 u, float s) { cblas_saxpy(3, s, u.arr, 1, v->arr, 1); }
inline static void vec4_add_scale_ip(vec4* v, vec4 u, float s) { cblas_saxpy(4, s, u.arr, 1, v->arr, 1); }

#define vec_sub_ip(v, u) _Generic((v), vec2*: vec2_sub_ip, \
                                       vec3*: vec3_sub_ip, \
                                       vec4*: vec4_sub_ip)(v, u)
inline static void vec2_sub_ip(vec2* v, vec2 u) { cblas_saxpy(2, -1, u.arr, 1, v->arr, 1); }
inline static void vec3_sub_ip(vec3* v, vec3 u) { cblas_saxpy(3, -1, u.arr, 1, v->arr, 1); }
inline static void vec4_sub_ip(vec4* v, vec4 u) { cblas_saxpy(4, -1, u.arr, 1, v->arr, 1); }

#define vec_dot(v, u) _Generic((v), vec2: vec2_dot, \
                                    vec3: vec3_dot, \
                                    vec4: vec4_dot)(v, u)
inline static float vec2_dot(vec2 v, vec2 u) { return cblas_sdot(2, v.arr, 1, u.arr, 1); }
inline static float vec3_dot(vec3 v, vec3 u) { return cblas_sdot(3, v.arr, 1, u.arr, 1); }
inline static float vec4_dot(vec4 v, vec4 u) { return cblas_sdot(4, v.arr, 1, u.arr, 1); }

/*****************************************************************************/

vec3 vec_from_dir(enum Direction dir);

#define vec_print(v) _Generic((v), vec2 : vec2_print , vec2*: vec2_printp, \
                                   vec3 : vec3_print , vec3*: vec3_printp, \
                                   vec4 : vec4_print , vec4*: vec4_printp)(v)
void vec2_print(vec2 v);
void vec2_printp(vec2* v);
void vec3_print(vec3 v);
void vec3_printp(vec3* v);
void vec4_print(vec4 v);
void vec4_printp(vec4* v);

#define vec_is_equal(v, u) _Generic((v), vec2: vec2_is_equal, \
                                         vec3: vec3_is_equal, \
                                         vec4: vec4_is_equal)(v, u)
inline static bool vec2_is_equal(vec2 v, vec2 u) { return is_equal(v.x, u.x) && is_equal(v.y, u.y); }
inline static bool vec3_is_equal(vec3 v, vec3 u) { return is_equal(v.x, u.x) && is_equal(v.y, u.y) && is_equal(v.z, u.z); }
inline static bool vec4_is_equal(vec4 v, vec4 u) { return is_equal(v.x, u.x) && is_equal(v.y, u.y) && is_equal(v.z, u.z) && is_equal(v.w, u.w); }

#define vec_normalise_ip(v) _Generic((v), vec2*: vec2_normalise_ip, \
                                          vec3*: vec3_normalise_ip, \
                                          vec4*: vec4_normalise_ip)(v)
inline static void vec2_normalise_ip(vec2* v) { vec_scale_ip(v, 1.0 / vec_mag(*v)); }
inline static void vec3_normalise_ip(vec3* v) { vec_scale_ip(v, 1.0 / vec_mag(*v)); }
inline static void vec4_normalise_ip(vec4* v) { vec_scale_ip(v, 1.0 / vec_mag(*v)); }

#define vec_clamp(v, max) _Generic((v), vec2*: vec2_clamp, \
                                        vec3*: vec3_clamp, \
                                        vec4*: vec4_clamp)(v, max)
inline static void vec2_clamp(vec2* v, float max) {
    float mag = vec_mag(*v);
    vec_normalise_ip(v);
    vec_scale_ip(v, MIN(mag, max));
}
inline static void vec3_clamp(vec3* v, float max) {
    float mag = vec_mag(*v);
    vec_normalise_ip(v);
    vec_scale_ip(v, MIN(mag, max));
}
inline static void vec4_clamp(vec4* v, float max) {
    float mag = vec_mag(*v);
    vec_normalise_ip(v);
    vec_scale_ip(v, MIN(mag, max));
}

#define vec_angle(v, u) _Generic((v), vec2: vec2_angle, \
                                      vec3: vec3_angle)(v, u)
inline static float vec2_angle(vec2 v, vec2 u) { return acos(vec_dot(v, u) / (vec_mag(v)*vec_mag(u))); }
inline static float vec3_angle(vec3 v, vec3 u) { return acos(vec_dot(v, u) / (vec_mag(v)*vec_mag(u))); }

inline static vec3 vec_cross(vec3 v, vec3 u) {
	return (vec3){ v.y * u.z - v.z * u.y,
	               v.z * u.x - v.x * u.z,
	               v.x * u.y - v.y * u.x, };
}

inline static float vec_triple(vec3 v, vec3 u, vec3 w) {
	return vec_dot(vec_cross(v, u), w);
}

#define vec_dist(v, u) _Generic((v), vec2: vec2_dist, \
                                     vec3: vec3_dist)(v, u)
inline static float vec2_dist(vec2 v, vec2 u) { return hypot(v.x - u.x, v.y - u.y); }
inline static float vec3_dist(vec3 v, vec3 u) { return sqrt(pow(v.x - u.x, 2) + pow(v.y - u.y, 2) + pow(v.z - u.z, 2)); }

inline static float vec_area(vec2 v)   { return v.w * v.h;       }

#define vec_volume(v) _Generic((v), vec3:  vec3_volume, \
                                   uvec3: uvec3_volume)(v)
inline static float  vec3_volume( vec3 v) { return v.w * v.h * v.d; }
inline static float uvec3_volume(uvec3 v) { return v.w * v.h * v.d; }

#endif
