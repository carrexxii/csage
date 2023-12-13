#ifndef MATHS_PGA_H
#define MATHS_PGA_H

#define VEC2(x, y)                    (Vec2){ x, y }
#define VEC3(x, y, z)                 (Vec3){ x, y, z }
#define VEC4(x, y, z, w)              (Vec4){ x, y, z, w }
#define SCALAR(a)                     (float)(a)
#define VEC(x, y, z, w)               (Vec){ x, y, z, w }
#define BIVEC(E1, E2, E3, e1, e2, e3) (Bivec){ E1, E2, E3, e1, e2, e3 }
#define BIVECV(v1, v2)                (Bivec){ v1.x, v1.y, v1.z, v2.x, v2.y, v2.z }
#define TRIVEC(x, y, z, w)            (Trivec){ x, y, z, w }
#define PSS(a)                        (PsS){ .e0123 = (a) }

typedef union Vec2 { struct { float x, y;       }; float arr[2]; vec2 v; } Vec2;
typedef union Vec3 { struct { float x, y, z;    }; float arr[3]; vec3 v; } Vec3;
typedef union Vec4 { struct { float x, y, z, w; }; float arr[4]; vec4 v; } Vec4;

typedef union Mat4x4 {
	mat4 m;
	struct { Vec4 r1, r2, r3, r4; };
	struct { float m11, m12, m13, m14,
	               m21, m22, m23, m24,
	               m31, m32, m33, m34,
	               m41, m42, m43, m44; };
	float arr[16];
} Mat4x4;

typedef union Point {
	struct { float  x,  y,  z; };
	struct { float E1, E2, E3; };
	float arr[3];
} Point;

typedef float Scalar;
typedef union Vec {
	struct { float  x,  y,  z,  w; };
	struct { float e1, e2, e3, e0; };
	float arr[4];
} Vec;
typedef union Bivec {
	struct { float e01, e02, e03, e23, e31, e12; };
	struct { float  wx,  wy,  wz,  yz,  zx,  xy; };
	struct { Vec3 m; Vec3 d; };
	struct { Vec3 V; Vec3 E; };
	float arr[6];
} Bivec;
typedef union Trivec {
	struct { float e032, e013, e021, e123; };
	struct { float    x,    y,    z,    w; };
	float arr[4];
} Trivec;
typedef union PsS { float e0123; } PsS;

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

/* negate: a -> -a */
#define negate(a) _Generic(a,  \
		Vec2  : negate_vec2,   \
		Vec3  : negate_vec3,   \
		Vec4  : negate_vec4,   \
		Scalar: negate_scalar, \
		Vec   : negate_vec,    \
		Bivec : negate_bivec,  \
		Trivec: negate_trivec, \
		PsS   : negate_pss     \
	)(a)
inline static Vec2 negate_vec2(Vec2 v) { return VEC2(-v.x, -v.y);             }
inline static Vec3 negate_vec3(Vec3 v) { return VEC3(-v.x, -v.y, -v.z);       }
inline static Vec4 negate_vec4(Vec4 v) { return VEC4(-v.x, -v.y, -v.z, -v.w); }
inline static Scalar negate_scalar(Scalar a) { return -a; }
inline static Vec negate_vec(Vec a) {
	return (Vec){
		.e0 = -a.e0,
		.e1 = -a.e1,
		.e2 = -a.e2,
		.e3 = -a.e3,
	};
}
inline static Bivec negate_bivec(Bivec a) {
	return (Bivec){
		.e01 = -a.e01,
		.e02 = -a.e02,
		.e03 = -a.e03,
		.e12 = -a.e12,
		.e23 = -a.e23,
		.e31 = -a.e31,
	};
}
inline static Trivec negate_trivec(Trivec a) {
	return (Trivec){
		.e021 = -a.e021,
		.e013 = -a.e013,
		.e032 = -a.e032,
		.e123 = -a.e123,
	};
}
inline static PsS negate_pss(PsS a) {
	return (PsS){
		.e0123 = -a.e0123,
	};
}

/* reverse: a -> ~a */
#define reverse(a) _Generic(a,                                               \
		Vec   : reverse_vec   , Bivec: reverse_bivec, Trivec: reverse_trivec, \
		Scalar: reverse_scalar, PsS  : reverse_pss                             \
	)(a)
inline static Scalar reverse_scalar(Scalar a) { return a; }
inline static Vec    reverse_vec(Vec a)       { return a; }
inline static PsS    reverse_pss(PsS a)       { return a; }
inline static Bivec reverse_bivec(Bivec a) {
	return (Bivec){
		.e23 = -a.e23, .e31 = -a.e31, .e12 = -a.e12,
		.e01 = -a.e01, .e02 = -a.e02, .e03 = -a.e03,
	};
}
inline static Trivec reverse_trivec(Trivec a) {
	return (Trivec){
		.e032 = -a.e032, .e013 = -a.e013, .e021 = -a.e021,
		.e123 = -a.e123,
	};
}

/* dual: a -> a* */
#define dual(a) _Generic(a,                                                  \
		Vec   : dual_of_vec   , Bivec: dual_of_bivec, Trivec: dual_of_trivec, \
		Scalar: dual_of_scalar, PsS  : dual_of_pss                             \
	)(a)
inline static PsS    dual_of_scalar(Scalar a) { return (PsS){ .e0123 = a, }; }
inline static Scalar dual_of_pss(PsS a)       { return a.e0123;              }
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

/* wedge: a, b -> a∧b */
#define wedge(_a, _b) _Generic(_a,                        \
		Scalar: _Generic(_b, Scalar: scalar_wedge_scalar, \
		                     Vec   : scalar_wedge_vec,    \
		                     Bivec : scalar_wedge_bivec,  \
		                     Trivec: scalar_wedge_trivec, \
		                     PsS   : scalar_wedge_pss     \
		),                                                \
		Vec: _Generic(_b, Scalar: vec_wedge_scalar,       \
		                  Vec   : vec_wedge_vec,          \
		                  Bivec : vec_wedge_bivec,        \
		                  Trivec: vec_wedge_trivec        \
		),                                                \
		Bivec: _Generic(_b, Scalar: bivec_wedge_scalar,   \
		                    Vec   : bivec_wedge_vec,      \
		                    Bivec : bivec_wedge_bivec     \
		)                                                 \
	)(_a, _b)
// Trivec: _Generic(_b, Scalar: scalar_wedge_trivec, 
//                      Vec   : trivec_wedge_vec     
// )                                                 
inline static Scalar scalar_wedge_scalar(Scalar a, Scalar b) {
	return a*b;
}
inline static Vec scalar_wedge_vec(Scalar a, Vec b) {
	return (Vec){
		.e0 = b.e0*a,
		.e1 = b.e1*a,
		.e2 = b.e2*a,
		.e3 = b.e3*a,
	};
}
inline static Bivec scalar_wedge_bivec(Scalar a, Bivec b) {
	return (Bivec){
		.e01 = b.e01*a,
		.e02 = b.e02*a,
		.e03 = b.e03*a,
		.e12 = b.e12*a,
		.e23 = b.e23*a,
		.e31 = b.e31*a,
	};
}
inline static Trivec scalar_wedge_trivec(Scalar a, Trivec b) {
	return (Trivec){
		.e013 = b.e013*a,
		.e021 = b.e021*a,
		.e032 = b.e032*a,
		.e123 = b.e123*a,
	};
}
inline static PsS scalar_wedge_pss(Scalar a, PsS b) {
	return (PsS){
		.e0123 = b.e0123*a,
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
		.e123 = -a.e1*b.e12 + a.e2*b.e31 + a.e3*b.e12,
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
inline static PsS    pss_wedge_scalar(PsS a, Scalar b)       { return scalar_wedge_pss(b, a);         }
inline static Vec    vec_wedge_scalar(Vec a, Scalar b)       { return scalar_wedge_vec(b, a);         }
inline static Bivec  bivec_wedge_scalar(Bivec a, Scalar b)   { return scalar_wedge_bivec(b, a);       }
inline static Trivec trivec_wedge_scalar(Trivec a, Scalar b) { return scalar_wedge_trivec(b, a);      }
inline static Trivec bivec_wedge_vec(Bivec a, Vec b)         { return vec_wedge_bivec(b, a);          }
inline static PsS    trivec_wedge_vec(Trivec a, Vec b)       { return negate(vec_wedge_trivec(b, a)); }

/* inner: a, b -> a⋅b */
#define inner(a, b) _Generic(a,                          \
		Scalar: _Generic(b, Scalar: scalar_inner_scalar, \
		                    Vec   : scalar_inner_vec,    \
		                    Bivec : scalar_inner_bivec,  \
		                    Trivec: scalar_inner_trivec  \
		),                                               \
		Vec: _Generic(b, Scalar: vec_inner_scalar,       \
		                 Vec   : vec_inner_vec,          \
		                 Bivec : vec_inner_bivec,        \
		                 Trivec: vec_inner_trivec        \
		),                                               \
		Bivec: _Generic(b, Scalar: bivec_inner_scalar,   \
		                   Vec   : bivec_inner_vec,      \
		                   Bivec : bivec_inner_bivec,    \
		                   Trivec: bivec_inner_trivec    \
		),                                               \
		Trivec: _Generic(b, Scalar: trivec_inner_scalar, \
		                    Vec   : trivec_inner_vec,    \
		                    Bivec : trivec_inner_bivec,  \
		                    Trivec: trivec_inner_trivec  \
		)                                                \
	)(a, b)
inline static Scalar scalar_inner_scalar(Scalar a, Scalar b) { return a*b; }
inline static Vec scalar_inner_vec(Scalar a, Vec b) {
	return (Vec){
		.e0 = b.e0*a,
		.e1 = b.e1*a,
		.e2 = b.e2*a,
		.e3 = b.e3*a,
	};
}
inline static Bivec scalar_inner_bivec(Scalar a, Bivec b) {
	return (Bivec){
		.e01 = b.e01*a,
		.e02 = b.e02*a,
		.e03 = b.e03*a,
		.e12 = b.e12*a,
		.e31 = b.e31*a,
		.e23 = b.e23*a,
	};
}
inline static Trivec scalar_inner_trivec(Scalar a, Trivec b) {
	return (Trivec){
		.e013 = b.e013*a,
		.e032 = b.e032*a,
		.e021 = b.e021*a,
		.e123 = b.e123*a,
	};
}
inline static Scalar vec_inner_vec(Vec a, Vec b) {
	return a.e1*b.e1 + a.e2*b.e2 + a.e3*b.e3;
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
inline static Scalar bivec_inner_bivec(Bivec a, Bivec b) {
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
inline static Scalar trivec_inner_trivec(Trivec a, Trivec b) {
	return -a.e123*b.e123;
}
inline static Vec    vec_inner_scalar(Vec a, Scalar b)       { return scalar_inner_vec(b, a);        }
inline static Bivec  bivec_inner_scalar(Bivec a, Scalar b)   { return scalar_inner_bivec(b, a);      }
inline static Vec    bivec_inner_vec(Bivec a, Vec b)         { return negate(vec_inner_bivec(b, a)); }
inline static Trivec trivec_inner_scalar(Trivec a, Scalar b) { return scalar_inner_trivec(b, a);     }
inline static Bivec  trivec_inner_vec(Trivec a, Vec b)       { return vec_inner_trivec(b, a);        }
inline static Vec    trivec_inner_bivec(Trivec a, Bivec b)   { return bivec_inner_trivec(b, a);      }

#define join(a, b) _Generic(a,                        \
		Trivec: _Generic(b, Trivec: trivec_join_trivec \
		)                                               \
	)(a, b)
/* a∨b = (e032 + e013 + e021 + e123)∨(e032 + e013 + e021 + e123) */
/* a∨b = ⋆-(⋆a∧⋆b)
 *     = (b - a)I3 - e0(a∧b)I3
 *     = 
 */
/* a∨b = (a*∧b*)-*
 *     = ((-e0 - e1 - e2 - e3)∧(-e0 - e1 - e2 - e3))*
 *     = ((e0 + e1 + e2 + e3)∧(e0 + e1 + e2 + e3))*
 */
inline static Bivec trivec_join_trivec(Trivec a, Trivec b) {
	Vec ad = dual(a);
	Vec bd = dual(b);
	Bivec r =  (Bivec){
		.e01 = ad.e0*bd.e1 - ad.e1*bd.e0,
		.e02 = ad.e0*bd.e2 - ad.e2*bd.e0,
		.e03 = ad.e0*bd.e3 - ad.e3*bd.e0,
		.e23 = ad.e2*bd.e3 - ad.e3*bd.e2,
		.e31 = ad.e3*bd.e1 - ad.e1*bd.e3,
		.e12 = ad.e1*bd.e2 - ad.e2*bd.e1,
	};
	return dual(r);
}

#define pga_print(a) _Generic(a,                                       \
		Vec2  : print_vec2  , Vec3 : print_vec3 , Vec4  : print_vec4  , \
		Vec   : print_vec   , Bivec: print_bivec, Trivec: print_trivec,  \
		Scalar: print_scalar, PsS  : print_pss                            \
	)(a)
static void print_vec2(Vec2 v) { printf("vec2: %6.3f, %6.3f\n", v.x, v.y);                                   }
static void print_vec3(Vec3 v) { printf("vec3: %6.3f, %6.3f, %6.3f\n", v.x, v.y, v.z);                       }
static void print_vec4(Vec4 v) { printf("vec4: %6.3f, %6.3f, %6.3f, %6.3f\n", v.x, v.y, v.z, v.w);           }
static void print_vec(Vec a)   { printf("Vector: %6.3fe1, %6.3fe2, %6.3fe3, %6.3fe0\n", a.x, a.y, a.z, a.w); }
static void print_bivec(Bivec a) {
	printf("Bivector: %6.3fe23, %6.3fe31, %6.3fe12, %6.3fe01, %6.3fe02, %6.3fe03\n",
	       a.e23, a.e31, a.e12, a.e01, a.e02, a.e03);
}
static void print_trivec(Trivec a) {
	printf("Trivector: %6.3fe032, %6.3fe013, %6.3fe021, %6.3fe123\n",
	       a.e032, a.e013, a.e021, a.e123);
}
static void print_scalar(Scalar a) { printf("Scalar: %6.3f\n", a);                  }
static void print_pss(PsS a)       { printf("Pseudoscalar: %6.3fe0123\n", a.e0123); }

#endif
