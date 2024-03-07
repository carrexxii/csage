#ifndef MATHS_PGA_H
#define MATHS_PGA_H

#include <stdio.h>
#include <math.h>

#include "types.h"
#include "la.h"
#include "macros.h"

static const Vec e0 = { .e0 = 1.0f };
static const Vec e1 = { .e1 = 1.0f };
static const Vec e2 = { .e2 = 1.0f };
static const Vec e3 = { .e3 = 1.0f };
static const Bivec e01 = { .e01 = 1.0f };
static const Bivec e02 = { .e02 = 1.0f };
static const Bivec e03 = { .e03 = 1.0f };
static const Bivec e12 = { .e12 = 1.0f };
static const Bivec e31 = { .e31 = 1.0f };
static const Bivec e23 = { .e23 = 1.0f };
static const Trivec e021 = { .e021 = 1.0f };
static const Trivec e032 = { .e032 = 1.0f };
static const Trivec e013 = { .e013 = 1.0f };
static const Trivec e123 = { .e123 = 1.0f };
static const PsS e0123 = { .e0123 = 1.0f };

/* -------------------------------------------------------------------- */

inline static float reverse_scalar(float a) { return a; }
inline static Vec   reverse_vec(Vec a)      { return a; }
inline static PsS   reverse_pss(PsS a)      { return a; }
inline static Bivec reverse_bivec(Bivec a) {
	return (Bivec){
		.e23 = -a.e23, .e31 = -a.e31, .e12 = -a.e12,
		.e01 = -a.e01, .e02 = -a.e02, .e03 = -a.e03,
	};
}
inline static Trivec reverse_trivec(Trivec a) {
	return (Trivec){
		.e032 = -a.e032,
		.e013 = -a.e013,
		.e021 = -a.e021,
		.e123 = -a.e123,
	};
}

/* -------------------------------------------------------------------- */

inline static PsS   dual_of_scalar(float a) { return (PsS){ .e0123 = a, }; }
inline static float dual_of_pss(PsS a)      { return a.e0123;              }
inline static Trivec dual_of_vec(Vec a) {
	return (Trivec){
		.e123 = a.e0,
		.e032 = a.e1,
		.e013 = a.e2,
		.e021 = a.e3,
	};
}
inline static Bivec dual_of_bivec(Bivec a) {
	return (Bivec){
		.e01 = a.e23,
		.e02 = a.e31,
		.e03 = a.e12,
		.e23 = a.e01,
		.e31 = a.e02,
		.e12 = a.e03,
	};
}
inline static Vec dual_of_trivec(Trivec a) {
	return (Vec){
		.e0 = a.e123,
		.e1 = a.e032,
		.e2 = a.e013,
		.e3 = a.e021,
	};
}

/* -------------------------------------------------------------------- */

inline static PsS    hodge_of_scalar(float a) { return dual_of_scalar(a); }
inline static float  hodge_of_pss(PsS a)      { return dual_of_pss(a);    }
inline static Trivec hodge_of_vec(Vec a)      { return dual_of_vec(a);    }
inline static Bivec  hodge_of_bivec(Bivec a)  { return dual_of_bivec(a);  }
inline static Vec hodge_of_trivec(Trivec a) {
	return (Vec){
		.e0 = -a.e123,
		.e1 = -a.e032,
		.e2 = -a.e013,
		.e3 = -a.e021,
	};
}

/* -------------------------------------------------------------------- */

inline static float scalar_add_scalar(float a, float b) { return a + b;                               }
inline static PsS   scalar_add_pss(float a, PsS b)      { return (PsS){ .e0123 = a + b.e0123 };       }
inline static PsS   pss_add_pss(PsS a, PsS b)           { return (PsS){ .e0123 = a.e0123 + b.e0123 }; }
inline static Vec scalar_add_vec(float a, Vec b) {
	return (Vec){
		.e1 = b.e1 + a,
		.e2 = b.e2 + a,
		.e3 = b.e3 + a,
		.e0 = b.e0 + a,
	};
}
inline static Bivec scalar_add_bivec(float a, Bivec b) {
	return (Bivec){
		.e01 = b.e01 + a,
		.e02 = b.e02 + a,
		.e03 = b.e03 + a,
		.e23 = b.e23 + a,
		.e31 = b.e31 + a,
		.e12 = b.e12 + a,
	};
}
inline static Trivec scalar_add_trivec(float a, Trivec b) {
	return (Trivec){
		.e032 = b.e032 + a,
		.e013 = b.e013 + a,
		.e021 = b.e021 + a,
		.e123 = b.e123 + a,
	};
}
inline static Vec vec_add_vec(Vec a, Vec b) {
	return (Vec){
		.e1 = a.e1 + b.e1,
		.e2 = a.e2 + b.e2,
		.e3 = a.e3 + b.e3,
		.e0 = a.e0 + b.e0,
	};
}
inline static Bivec bivec_add_bivec(Bivec a, Bivec b) {
	return (Bivec){
		.e01 = a.e01 + b.e01,
		.e02 = a.e02 + b.e02,
		.e03 = a.e03 + b.e03,
		.e23 = a.e23 + b.e23,
		.e31 = a.e31 + b.e31,
		.e12 = a.e12 + b.e12,
	};
}
inline static Trivec trivec_add_trivec(Trivec a, Trivec b) {
	return (Trivec){
		.e032 = a.e032 + b.e032,
		.e013 = a.e013 + b.e013,
		.e021 = a.e021 + b.e021,
		.e123 = a.e123 + b.e123,
	};
}
inline static Vec    vec_add_scalar(Vec a, float b)       { return scalar_add_vec(b, a);    }
inline static Bivec  bivec_add_scalar(Bivec a, float b)   { return scalar_add_bivec(b, a);  }
inline static Trivec trivec_add_scalar(Trivec a, float b) { return scalar_add_trivec(b, a); }
inline static PsS    pss_add_scalar(PsS a, float b)       { return scalar_add_pss(b, a);    }

/* -------------------------------------------------------------------- */

inline static float scalar_multiply_scalar(float a, float b) { return a * b;                               }
inline static PsS   scalar_multiply_pss(float a, PsS b)      { return (PsS){ .e0123 = a * b.e0123 };       }
inline static PsS   pss_multiply_pss(PsS a, PsS b)           { return (PsS){ .e0123 = a.e0123 * b.e0123 }; }
inline static Vec scalar_multiply_vec(float a, Vec b) {
	return (Vec){
		.e1 = b.e1 * a,
		.e2 = b.e2 * a,
		.e3 = b.e3 * a,
		.e0 = b.e0 * a,
	};
}
inline static Bivec scalar_multiply_bivec(float a, Bivec b) {
	return (Bivec){
		.e01 = b.e01 * a,
		.e02 = b.e02 * a,
		.e03 = b.e03 * a,
		.e23 = b.e23 * a,
		.e31 = b.e31 * a,
		.e12 = b.e12 * a,
	};
}
inline static Trivec scalar_multiply_trivec(float a, Trivec b) {
	return (Trivec){
		.e032 = b.e032 * a,
		.e013 = b.e013 * a,
		.e021 = b.e021 * a,
		.e123 = b.e123 * a,
	};
}
inline static Vec vec_multiply_vec(Vec a, Vec b) {
	return (Vec){
		.e1 = a.e1 * b.e1,
		.e2 = a.e2 * b.e2,
		.e3 = a.e3 * b.e3,
		.e0 = a.e0 * b.e0,
	};
}
inline static Bivec bivec_multiply_bivec(Bivec a, Bivec b) {
	return (Bivec){
		.e01 = a.e01 * b.e01,
		.e02 = a.e02 * b.e02,
		.e03 = a.e03 * b.e03,
		.e23 = a.e23 * b.e23,
		.e31 = a.e31 * b.e31,
		.e12 = a.e12 * b.e12,
	};
}
inline static Trivec trivec_multiply_trivec(Trivec a, Trivec b) {
	return (Trivec){
		.e032 = a.e032 * b.e032,
		.e013 = a.e013 * b.e013,
		.e021 = a.e021 * b.e021,
		.e123 = a.e123 * b.e123,
	};
}
inline static Vec    vec_multiply_scalar(Vec a, float b)       { return scalar_multiply_vec(b, a);    }
inline static Bivec  bivec_multiply_scalar(Bivec a, float b)   { return scalar_multiply_bivec(b, a);  }
inline static Trivec trivec_multiply_scalar(Trivec a, float b) { return scalar_multiply_trivec(b, a); }
inline static PsS    pss_multiply_scalar(PsS a, float b)       { return scalar_multiply_pss(b, a);    }

/* -------------------------------------------------------------------- */

// Trivec: _Generic(_b, Scalar: scalar_wedge_trivec,
//                      Vec   : trivec_wedge_vec
// )
inline static float scalar_wedge_scalar(float a, float b) {
	return a * b;
}
inline static Vec scalar_wedge_vec(float a, Vec b) {
	return (Vec){
		.e0 = b.e0 * a,
		.e1 = b.e1 * a,
		.e2 = b.e2 * a,
		.e3 = b.e3 * a,
	};
}
inline static Bivec scalar_wedge_bivec(float a, Bivec b) {
	return (Bivec){
		.e01 = b.e01 * a,
		.e02 = b.e02 * a,
		.e03 = b.e03 * a,
		.e12 = b.e12 * a,
		.e23 = b.e23 * a,
		.e31 = b.e31 * a,
	};
}
inline static Trivec scalar_wedge_trivec(float a, Trivec b) {
	return (Trivec){
		.e013 = b.e013 * a,
		.e021 = b.e021 * a,
		.e032 = b.e032 * a,
		.e123 = b.e123 * a,
	};
}
inline static PsS scalar_wedge_pss(float a, PsS b) {
	return (PsS){
		.e0123 = b.e0123 * a,
	};
}
/* p1∧p2 = (e0 + e1 + e2 + e3)∧(e0 + e1 + e2 + e3) */
inline static Bivec vec_wedge_vec(Vec a, Vec b) {
	return (Bivec){
		.e01 = a.e0*b.e1 - a.e1*b.e0,
		.e02 = a.e0*b.e2 - a.e2*b.e0,
		.e03 = a.e0*b.e3 - a.e3*b.e0,
		.e12 = a.e1*b.e2 - a.e2*b.e1,
		.e31 = a.e3*b.e1 - a.e1*b.e3,
		.e23 = a.e2*b.e3 - a.e3*b.e2,
	};
}
/* p∧l = (e0 + e1 + e2 + e3)∧(e23, e31, e12, e01, e02, e03) */
inline static Trivec vec_wedge_bivec(Vec a, Bivec b) {
	return (Trivec){
		.e032 = -a.e0*b.e23 + a.e2*b.e03 - a.e3*b.e02,
		.e013 = -a.e0*b.e31 - a.e1*b.e03 + a.e3*b.e01,
		.e021 = -a.e0*b.e12 + a.e1*b.e02 - a.e2*b.e01,
		.e123 = a.e1*b.e23 + a.e2*b.e31 + a.e3*b.e12,
	};
}
/* p∧P = (e0 + e1 + e2 + e3)∧(e032 + e013 + e021 + e123) */
inline static PsS vec_wedge_trivec(Vec a, Trivec b) {
	return (PsS){
		.e0123 = a.e0*b.e123 + a.e1*b.e032 + a.e2*b.e013 + a.e3*b.e021,
	};
}
/* l1∧l2 = (e23, e31, e12, e01, e02, e03)∧(e23, e31, e12, e01, e02, e03) */
inline static PsS bivec_wedge_bivec(Bivec a, Bivec b) {
	return (PsS){
		.e0123 = a.e23*b.e01 + a.e31*b.e02 + a.e12*b.e03 +
		         a.e01*b.e23 + a.e02*b.e31 + a.e03*b.e12,
	};
}
inline static PsS    pss_wedge_scalar(PsS a, float b)       { return scalar_wedge_pss(b, a);                  }
inline static Vec    vec_wedge_scalar(Vec a, float b)       { return scalar_wedge_vec(b, a);                  }
inline static Bivec  bivec_wedge_scalar(Bivec a, float b)   { return scalar_wedge_bivec(b, a);                }
inline static Trivec trivec_wedge_scalar(Trivec a, float b) { return scalar_wedge_trivec(b, a);               }
inline static Trivec bivec_wedge_vec(Bivec a, Vec b)        { return vec_wedge_bivec(b, a);                   }
inline static PsS    trivec_wedge_vec(Trivec a, Vec b)      { return multiply(vec_wedge_trivec(b, a), -1.0f); }

/* -------------------------------------------------------------------- */

inline static float scalar_inner_scalar(float a, float b) { return a * b;             }
inline static float pss_inner_pss(PsS a, PsS b)           { return a.e0123 * b.e0123; }
inline static Vec scalar_inner_vec(float a, Vec b) {
	return (Vec){
		.e0 = b.e0 * a,
		.e1 = b.e1 * a,
		.e2 = b.e2 * a,
		.e3 = b.e3 * a,
	};
}
inline static Bivec scalar_inner_bivec(float a, Bivec b) {
	return (Bivec){
		.e01 = b.e01 * a,
		.e02 = b.e02 * a,
		.e03 = b.e03 * a,
		.e12 = b.e12 * a,
		.e31 = b.e31 * a,
		.e23 = b.e23 * a,
	};
}
inline static Trivec scalar_inner_trivec(float a, Trivec b) {
	return (Trivec){
		.e013 = b.e013 * a,
		.e032 = b.e032 * a,
		.e021 = b.e021 * a,
		.e123 = b.e123 * a,
	};
}
inline static float vec_inner_vec(Vec a, Vec b) {
	return a.e1*b.e1 + a.e2*b.e2 + a.e3*b.e3;
}
inline static float vec3_inner_vec3(Vec3 a, Vec3 b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}
/* p⋅l = (e0 + e1 + e2 + e3)⋅(e01 + e02 + e03 + e12 + e31 + e23) */
inline static Vec vec_inner_bivec(Vec a, Bivec b) {
	return (Vec){
		.e0 = -a.e1*b.e01 - a.e2*b.e02 - a.e3*b.e03,
		.e1 = a.e3*b.e31 - a.e2*b.e12,
		.e2 = a.e1*b.e12 - a.e3*b.e23,
		.e3 = a.e2*b.e23 - a.e1*b.e31,
	};
}
/* p⋅P = (e0 + e1 + e2 + e3)⋅(e032 + e013 + e021 + e123) */
inline static Bivec vec_inner_trivec(Vec a, Trivec b) {
	return (Bivec){
		.e01 = a.e3*b.e013 - a.e2*b.e021,
		.e02 = a.e1*b.e021 - a.e3*b.e032,
		.e03 = a.e2*b.e032 - a.e1*b.e013,
		.e12 = a.e3*b.e123,
		.e23 = a.e1*b.e123,
		.e31 = a.e2*b.e123,
	};
}
/* l1⋅l2 = (e01 + e02 + e03 + e12 + e31 + e23)⋅(e01 + e02 + e03 + e12 + e31 + e23) */
inline static float bivec_inner_bivec(Bivec a, Bivec b) {
	return -a.e12*b.e12 - a.e31*b.e31 - a.e23*b.e23;
}
/* l⋅P = (e01 + e02 + e03 + e12 + e31 + e23)⋅(e032 + e013 + e021 + e123) */
inline static Vec bivec_inner_trivec(Bivec a, Trivec b) {
	return (Vec){
		.e0 = a.e12*b.e021 + a.e31*b.e013 + a.e23*b.e032,
		.e1 = -a.e23*b.e123,
		.e2 = -a.e31*b.e123,
		.e3 = -a.e12*b.e123,
	};
}
/* P1⋅P2 = (e032 + e013 + e021 + e123)⋅(e032 + e013 + e021 + e123) */
inline static float trivec_inner_trivec(Trivec a, Trivec b) {
	return -a.e123 * b.e123;
}
inline static Vec    vec_inner_scalar(Vec a, float b)       { return scalar_inner_vec(b, a);                 }
inline static Bivec  bivec_inner_scalar(Bivec a, float b)   { return scalar_inner_bivec(b, a);               }
inline static Vec    bivec_inner_vec(Bivec a, Vec b)        { return multiply(vec_inner_bivec(b, a), -1.0f); }
inline static Trivec trivec_inner_scalar(Trivec a, float b) { return scalar_inner_trivec(b, a);              }
inline static Bivec  trivec_inner_vec(Trivec a, Vec b)      { return vec_inner_trivec(b, a);                 }
inline static Vec    trivec_inner_bivec(Trivec a, Bivec b)  { return bivec_inner_trivec(b, a);               }

/* -------------------------------------------------------------------- */

inline static float scalar_norm(float a)  { return sqrtf(norm2(a)); }
inline static float vec_norm(Vec a)       { return sqrtf(norm2(a)); }
inline static float vec3_norm(Vec3 a)     { return sqrtf(norm2(a)); }
inline static float bivec_norm(Bivec a)   { return sqrtf(norm2(a)); }
inline static float trivec_norm(Trivec a) { return sqrtf(norm2(a)); }
inline static float pss_norm(PsS a)       { return sqrtf(norm2(a)); }

/* -------------------------------------------------------------------- */

inline static Vec    vec_normalized(Vec a)       { return multiply(a, 1.0f / norm(a)); }
inline static Bivec  bivec_normalized(Bivec a)   { return multiply(a, 1.0f / norm(a)); }
inline static Trivec trivec_normalized(Trivec a) { return multiply(a, 1.0f / norm(a)); }

/* -------------------------------------------------------------------- */

/* a∨b = ⋆-(⋆a∧⋆b)
 *     = ⋆-(e01 + e02 + e03 + e12 + e31 + e23)∧(-e0 - e1 - e2 - e3)
 *     = -⋆(e01 + e02 + e03 + e12 + e31 + e23)∧(-e0 - e1 - e2 - e3)
 */
inline static Vec bivec_join_trivec(Bivec a, Trivec b) {
	return multiply(hodge(wedge(hodge(a), hodge(b))), -1.0f);
}
/* a∨b = ⋆-(⋆a∧⋆b)
 *     = ⋆-((e0 + e1 + e2 + e3)∧(e0 + e1 + e2 + e3))
 *     = -⋆((e0 + e1 + e2 + e3)∧(e0 + e1 + e2 + e3))
 */
inline static Bivec trivec_join_trivec(Trivec a, Trivec b) {
	return multiply(hodge(wedge(hodge(a), hodge(b))), -1.0f);
}

/* -------------------------------------------------------------------- */

static void print_vec2(Vec2 v) { printf("vec2: %6.3f, %6.3f\n", v.x, v.y);                                       }
static void print_vec3(Vec3 v) { printf("vec3: %6.3f, %6.3f, %6.3f\n", v.x, v.y, v.z);                           }
static void print_vec4(Vec4 v) { printf("vec4: %6.3f, %6.3f, %6.3f, %6.3f\n", v.x, v.y, v.z, v.w);               }
static void print_vec(Vec a)   { printf("Vector: %6.3fe0, %6.3fe1, %6.3fe2, %6.3fe3\n", a.e0, a.e1, a.e2, a.e3); }
static void print_bivec(Bivec a) {
	printf("Bivector: %6.3fe01, %6.3fe02, %6.3fe03, %6.3fe23, %6.3fe31, %6.3fe12\n",
	       a.e01, a.e02, a.e03, a.e23, a.e31, a.e12);
}
static void print_trivec(Trivec a) {
	printf("Trivector: %6.3fe032, %6.3fe013, %6.3fe021, %6.3fe123\n",
	       a.e032, a.e013, a.e021, a.e123);
}
static void print_scalar(float a) { printf("Scalar: %6.3f\n", a);                  }
static void print_pss(PsS a)      { printf("Pseudoscalar: %6.3fe0123\n", a.e0123); }

#endif
