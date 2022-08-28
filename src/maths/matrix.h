#ifndef MATHS_MATRIX_H
#define MATHS_MATRIX_H

#include "vector.h"

#define CBLAS_MAT_ORDER CblasRowMajor
#define CBLAS_MAT_TRANS CblasNoTrans

typedef union {
	float arr[9];
 	struct { float m11, m12, m13,
	               m21, m22, m23,
	               m31, m32, m33; };
} mat3; static_assert(sizeof(mat3) == 36, "mat3");

typedef union {
	float arr[16];
	struct { float m11, m12, m13, m14,
	               m21, m22, m23, m24,
	               m31, m32, m33, m34,
	               m41, m42, m43, m44; };
} mat4; static_assert(sizeof(mat4) == 64, "mat4");

enum MatrixTransform {
	MTRANS_TRANSLATE,
	MTRANS_SCALE,
	MTRANS_ROTATE,
};

struct MatTrans {
	enum MatrixTransform type;
	union {
		vec3 v;
		struct {
			enum Axis axis;
			float angle;
		};
	};
}; static_assert(sizeof(struct MatTrans) == 16, "struct MatTrans");

#define MAT_ZERO { 0.0, 0.0, 0.0, 0.0, \
                   0.0, 0.0, 0.0, 0.0, \
                   0.0, 0.0, 0.0, 0.0, \
                   0.0, 0.0, 0.0, 0.0, }
#define MAT3_IDENT { 1.0, 0.0, 0.0, \
                     0.0, 1.0, 0.0, \
                     0.0, 0.0, 1.0, }
#define MAT4_IDENT { 1.0, 0.0, 0.0, 0.0, \
                     0.0, 1.0, 0.0, 0.0, \
                     0.0, 0.0, 1.0, 0.0, \
                     0.0, 0.0, 0.0, 1.0, }

#define MAT_NEW_TRANSLATION(v) _Generic((v),                         \
	vec2: ((mat3){ .m31 = v.x, .m32 = v.y, .m33 = 1.0             }), \
	vec3: ((mat4){ .m41 = v.x, .m42 = v.y, .m43 = v.z, .m44 = 1.0 }))
#define MAT_NEW_SCALING(v) _Generic((v),                             \
	vec2: ((mat3){ .m11 = v.x, .m22 = v.y, .m33 = 1.0             }), \
	vec3: ((mat4){ .m11 = v.x, .m22 = v.y, .m33 = v.z, .m44 = 1.0 }))

extern const mat3 MAT_I3;
extern const mat4 MAT_I4;

#define mat_print(a) _Generic((a), mat3*: mat_print_((a)->arr, 3), \
                                   mat4*: mat_print_((a)->arr, 4))
void mat_print_(const float* a, int dim);

/* a = b */
#define mat_copy(a, b) _Generic((a), mat3*: mat3_copy, \
                                     mat4*: mat4_copy)(a, b)
inline static void mat3_copy(mat3* restrict a, const mat3* restrict b) { memcpy(a->arr, b->arr, sizeof(mat3)); }
inline static void mat4_copy(mat4* restrict a, const mat4* restrict b) { memcpy(a->arr, b->arr, sizeof(mat4)); }

/* a += b */
#define mat_add_ip(a, b) _Generic((a), mat3*: mat3_add_ip, \
                                       mat4*: mat4_add_ip)(a, b)
inline static void mat3_add_ip(mat3* restrict a, const mat3* restrict b) { cblas_saxpy(9 , 1, b->arr, 1, a->arr, 1); }
inline static void mat4_add_ip(mat4* restrict a, const mat4* restrict b) { cblas_saxpy(16, 1, b->arr, 1, a->arr, 1); }

/* a *= s */
#define mat_scale_ip(a, s) _Generic((a), mat3*: mat3_scale_ip, \
                                         mat4*: mat4_scale_ip)(a, s)
inline static void mat3_scale_ip(mat3* a, float s) { cblas_sscal(9 , s, a->arr, 1); }
inline static void mat4_scale_ip(mat4* a, float s) { cblas_sscal(16, s, a->arr, 1); }

/* a = b*c */
#define mat_mul(a, b, c) _Generic((a), mat3*: mat3_mul, \
                                       mat4*: mat4_mul)(a, b, c)
inline static void mat3_mul(mat3* restrict a, const mat3* restrict b, mat3* restrict c) {
	cblas_sgemm(CBLAS_MAT_ORDER, CBLAS_MAT_TRANS, CBLAS_MAT_TRANS, 3, 3, 3, 1.0, b->arr, 3, c->arr, 3, 0.0, a->arr, 3);
}
inline static void mat4_mul(mat4* restrict a, const mat4* restrict b, mat4* restrict c) {
	cblas_sgemm(CBLAS_MAT_ORDER, CBLAS_MAT_TRANS, CBLAS_MAT_TRANS, 4, 4, 4, 1.0, b->arr, 4, c->arr, 4, 0.0, a->arr, 4);
}

/* a *= b */
#define mat_mul_ip(a, b) _Generic((a), mat3*: mat3_mul_ip, \
                                       mat4*: mat4_mul_ip)(a, b)
inline static void mat3_mul_ip(mat3* restrict a, const mat3* restrict b) {
	mat3 tmp;
	cblas_sgemm(CBLAS_MAT_ORDER, CBLAS_MAT_TRANS, CBLAS_MAT_TRANS, 3, 3, 3, 1.0, a->arr, 3, b->arr, 3, 0.0, tmp.arr, 3);
	mat_copy(a, &tmp);
}
inline static void mat4_mul_ip(mat4* restrict a, const mat4* restrict b) {
	mat4 tmp;
	cblas_sgemm(CBLAS_MAT_ORDER, CBLAS_MAT_TRANS, CBLAS_MAT_TRANS, 4, 4, 4, 1.0, a->arr, 4, b->arr, 4, 0.0, tmp.arr, 4);
	mat_copy(a, &tmp);
}

/* TODO: integrate into mat_mul */
/* a *= v */
#define mat_mul_v_ip(a, v) _Generic((a), mat3*: mat3_mul_v_ip, \
                                         mat4*: mat4_mul_v_ip, \
                                         vec4*: vec4_mul_mat4_ip)(a, v)
inline static void mat3_mul_v_ip(mat3* a, vec2 v) {
	cblas_sgemv(CBLAS_MAT_ORDER, CBLAS_MAT_TRANS, 3, 3, 1.0, a->arr, 3, v.arr, 1, 0.0, NULL, 1);
}
inline static void mat4_mul_v_ip(mat4* a, vec3 v) {
	cblas_sgemv(CBLAS_MAT_ORDER, CBLAS_MAT_TRANS, 4, 4, 1.0, a->arr, 4, v.arr, 1, 0.0, NULL, 1);
}
inline static void vec4_mul_mat4_ip(vec4* v, mat4* a) {
	vec4 u = VEC4P(v);
	v->x = a->m11*u.x + a->m12*u.y + a->m13*u.z + a->m14*u.w;
	v->y = a->m21*u.x + a->m22*u.y + a->m23*u.z + a->m24*u.w;
	v->z = a->m31*u.x + a->m32*u.y + a->m33*u.z + a->m34*u.w;
	v->w = a->m41*u.x + a->m42*u.y + a->m43*u.z + a->m44*u.w;
}

#define mat_set_x(a, x) _Generic((a), mat3*: a->m31 = x, mat4*: a->m41 = x)
#define mat_set_y(a, y) _Generic((a), mat3*: a->m32 = y, mat4*: a->m42 = y)
#define mat_set_z(a, z) _Generic((a),                    mat4*: a->m43 = z)

#define mat_get_pos(a) _Generic((a), mat3*: mat3_get_pos, \
                                     mat4*: mat4_get_pos)(a)
inline static vec2 mat3_get_pos(const mat3* a) { return (vec2){ a->m31, a->m32         }; }
inline static vec3 mat4_get_pos(const mat4* a) { return (vec3){ a->m41, a->m42, a->m43 }; }

/* TODO: remove memcpy */
#define mat_set_pos(a, v) _Generic((a), mat3*: mat3_set_pos,                    \
                                        mat4*: _Generic((v), vec3: mat4_set_pos, \
                                                             vec2: mat4_set_pos_v2))(a, v)
inline static void mat3_set_pos(mat3* a, vec2 v)    { memcpy(&a->m31, &v, sizeof(vec2)); }
inline static void mat4_set_pos(mat4* a, vec3 v)    { memcpy(&a->m41, &v, sizeof(vec3)); }
inline static void mat4_set_pos_v2(mat4* a, vec2 v) { memcpy(&a->m41, &v, sizeof(vec2)); }

#define mat_scale_v(a, v) _Generic((a), mat3*: mat3_scale_v,                      \
                                        mat4*: _Generic((v), vec2: mat4_scale_v2,  \
                                                             vec3: mat4_scale_v3,   \
                                                             vec4: mat4_scale_v4))(a, v)
inline static void mat3_scale_v(mat3* a, vec2 v) {
	a->m11 *= v.x;
	a->m22 *= v.y;
}
inline static void mat4_scale_v2(mat4* a, vec2 v) {
	a->m11 *= v.x;
	a->m22 *= v.y;
}
inline static void mat4_scale_v3(mat4* a, vec3 v) {
	a->m11 *= v.x;
	a->m22 *= v.y;
	a->m33 *= v.z;
}
inline static void mat4_scale_v4(mat4* a, vec4 v) {
	a->m11 *= v.x;
	a->m22 *= v.y;
	a->m33 *= v.z;
	a->m44 *= v.w;
}

#define mat_translate(a, v) _Generic((a), mat4*: mat4_translate)(a, v)
inline static void mat4_translate(mat4* a, vec3 v) {
	a->m41 += v.x;
	a->m42 += v.y;
	a->m43 += v.z;
}
#define mat_translate_x(a, x) _Generic((a), mat4*: mat4_translate_x)(a, x)
#define mat_translate_y(a, y) _Generic((a), mat4*: mat4_translate_y)(a, y)
#define mat_translate_z(a, z) _Generic((a), mat4*: mat4_translate_z)(a, z)
inline static void mat4_translate_x(mat4* a, float x) { a->m41 += x; }
inline static void mat4_translate_y(mat4* a, float y) { a->m42 += y; }
inline static void mat4_translate_z(mat4* a, float z) { a->m43 += z; }

#define mat_rotate(a, axis, θ) _Generic((a), mat4*: mat4_rotate)(a, axis, θ)
#define mat_rotate_x(a, θ) _Generic((a), mat4*: mat4_rotate)(a, X_AXIS, θ)
#define mat_rotate_y(a, θ) _Generic((a), mat4*: mat4_rotate)(a, Y_AXIS, θ)
#define mat_rotate_z(a, θ) _Generic((a), mat4*: mat4_rotate)(a, Z_AXIS, θ)
void mat4_rotate(mat4* a, enum Axis axis, float θ);

void mat_new_persp(mat4* mat, float ar, float fov, float zn, float zf);
void mat_new_ortho(mat4* mat, float top, float bot, float r, float l, float zn, float zf);

inline static struct MatTrans mat_new_trans(enum MatrixTransform type, enum Axis axis, float θ) {
	return (struct MatTrans){ .type = type, .axis = axis, .angle = θ };
}
inline static struct MatTrans mat_new_trans_v(enum MatrixTransform type, vec3 v) {
	return (struct MatTrans){ .type = type, .v = v };
}

#define mat_transform(a, tc, t) _Generic((a), mat3*: mat3_transform, \
                                              mat4*: mat4_transform)(a, tc, t)
void mat3_transform(mat3* a, struct MatTrans* ts, int tc);
void mat4_transform(mat4* a, struct MatTrans* ts, int tc);

#define mat_inv(a) _Generic((a), mat4*: mat4_inv)(a)
void mat4_inv(mat4* a);

void mat_transpose(mat4* a);
void mat_unproject(vec4* v, mat4* inv, vec4 vp);

#endif
