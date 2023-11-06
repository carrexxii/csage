#ifndef PGA_H
#define PGA_H

// typedef union Vec2 {
// 	struct { float e1; float e2; };
// 	struct { float  x; float  y; };
// 	struct { float  u; float  v; };
// 	float vec[2];
// } Vec2;
typedef union Vec3 {
	struct { float e1; float e2; float e3; };
	struct { float  x; float  y; float  z; };
	struct { float  r; float  g; float  b; };
	float vec[3];
} Vec3;
typedef union Vec4 {
	struct { float e1; float e2; float e3; float e4; };
	struct { float  r; float  g; float  b; float  a; };
	struct {
		union {
			struct { float x; float y; float z; };
			vec3 p;
		};
		float w;
	};
	float vec[4];
	// float vec __attribute__((vector_size(16)));
} Vec4;

// typedef union BiVec2 {
// 	float e12;
// 	float  xy;
// } BiVec2;
// typedef union BiVec3 {
// 	struct { float e23; float e31; float e12; };
// 	struct { float  yz; float  zx; float  xy; };
// 	float vec[3];
// } BiVec3;
typedef union BiVec {
	struct { float e41; float e42; float e43; float e23; float e31; float e12; };
	struct { float  wx; float  wy; float  wz; float  yz; float  zx; float  xy; };
	struct { float  vx; float  vy; float  vz; float  mx; float  my; float  mz; };
	struct { Vec3 v; Vec3 m; }; /* v = direction; m = moment */
	float vec[6];
} BiVec;

// typedef union TriVec3 {
// 	float e123;
// 	float  xyz;
// } TriVec3;
typedef union TriVec {
	struct { float e423; float e431; float e412; float e321; };
	struct { float  wyz; float  wzx; float  wxy; float  zyx; };
	struct { float    x; float    y; float    z; float    w; };
	struct {
		union {
			struct { float nx; float ny; float nz; };
			Vec3 n;
		};
		float d;
	};
	float vec[4];
} TriVec;

typedef float QuadVec;

typedef Vec3   Point;
typedef BiVec  Line;
typedef TriVec Plane;

/* -------------------------------------------------------------------- */

inline static float magnitude2(Vec3 v) { return v.x*v.x + v.y*v.y + v.z*v.z; }
inline static float magnitude(Vec3 v)  { return sqrtf(magnitude2(v));        }
inline static Vec3 normalized(Vec3 v) {
	float mag = magnitude(v);
	return (Vec3){ v.x/mag, v.y/mag, v.z/mag };
}
inline static void normalize(Vec3* v) {
	float mag = magnitude(*v);
	v->x /= mag;
	v->y /= mag;
	v->z /= mag;
}

#define vec_sum(v, u) _Generic((a), \
	Vec3: _Generic(u, Vec3: vec3_vec3_sum(v, u)))
inline static Vec3 vec3_vec3_sum(Vec3 v, Vec3 u) { return (Vec3){ v.x + u.x, v.y + u.y, v.z + u.z }; }

/* -------------------------------------------------------------------- */

#define vec_dot(v, u) _Generic(v, \
	Vec3: _Generic(u, Vec3: vec3_dot_vec3(v, u)))
inline static float vec3_dot_vec3(Vec3 v, Vec3 u) { return v.x*u.x + v.y*u.y + v.z*u.z; }

/* -------------------------------------------------------------------- */

/* vec_wedge(v, u) -> v ∧ u */
#define wedge(v, u) _Generic((v),                      \
	Vec3  : _Generic(u, Vec3  :  vec_wedge_vec(v, u),   \
	                    BiVec :  vec_wedge_bivec(v, u),  \
	                    TriVec:  vec_wedge_trivec(v, u)), \
	BiVec : _Generic(u, Vec3  :  vec_wedge_bivec(u, v),    \
	                    BiVec :  bivec_wedge_bivec(v, u),   \
	                    TriVec:  bivec_wedge_trivec(v, u)),  \
	TriVec: _Generic(u, Vec3  : -vec_wedge_trivec(u, v),      \
	                    BiVec :  bivec_wedge_trivec(u, v),     \
	                    TriVec:  trivec_wedge_trivec(v, u)))
/* p ∧ q = Line containing points p and q. Zero if p and q are coincident. */
inline static BiVec vec_wedge_vec(Vec3 p, Vec3 q) {
	return (BiVec){
		.wx = q.x - p.x,         /* e41 */
		.wy = q.y - p.y,         /* e42 */
		.wz = q.z - p.z,         /* e43 */
		.yz = p.y*q.z - p.z*q.y, /* e23 */
		.zx = p.z*q.x - p.x*q.z, /* e13 */
		.xy = p.x*q.y - p.y*q.x, /* e12 */
	};
}
/* p ∧ L = Plane containing line L and point p. Normal is zero if p lies in L.
 *   Assumes w = 1
 */
inline static TriVec vec_wedge_bivec(Vec3 p, BiVec L) {
	return (TriVec){
		.wyz =  L.vy*p.z - L.vz*p.y + L.mx,     /* !e1 */
		.wzx =  L.vz*p.x - L.vx*p.z + L.my,     /* !e2 */
		.wxy =  L.vx*p.y - L.vy*p.x + L.mz,     /* !e3 */
		.zyx = -L.mx*p.x - L.my*p.y - L.mz*p.z, /* !e4 */
	};
}
/* p ∧ f */
static inline QuadVec vec_wedge_trivec(Vec3 p, TriVec f) {
	return (QuadVec)(p.x*f.x + p.y*f.y + p.z*f.z + f.w);
}
/* L1 ∧ L2 */
static inline QuadVec bivec_wedge_bivec(BiVec L1, BiVec L2) {
	return (QuadVec)(-vec_dot(L1.v, L2.m) + vec_dot(L2.v, L1.m));
}
/* L ∧ f */
static inline Vec4 bivec_wedge_trivec(BiVec L, TriVec f) {
	return (Vec4){
		.x =  L.my*f.z - L.mz*f.y + L.vx*f.w,
		.y =  L.mz*f.x - L.mx*f.z + L.vy*f.w,
		.z =  L.mx*f.y - L.my*f.x + L.vz*f.w,
		.w = -L.vx*f.x - L.vy*f.y + L.vz*f.z,
	};
}
/* f ∧ g */
static inline BiVec trivec_wedge_trivec(TriVec f, TriVec g) {
	return (BiVec){
		.wx = f.y*g.z - f.z*g.y, /* e41 */
		.wy = f.z*g.x - f.x*g.z, /* e42 */
		.wz = f.x*g.y - f.y*g.x, /* e43 */
		.yz = f.w*g.x - f.x*g.w, /* e23 */
		.zx = f.w*g.y - f.y*g.w, /* e31 */
		.xy = f.w*g.z - f.z*g.w, /* e12 */
	};
}

/* -------------------------------------------------------------------- */

/* vec_antiwedge(v, u) -> v ∨ u */
#define antiwedge(v, u) _Generic((v),                          \
	TriVec: _Generic(u, TriVec: trivec_antiwedge_trivec(v, u)), \
	                    BiVec : bivec_antiwedge_trivec(u, v)),   \
	BiVec : _Generic(u, BiVec : bivec_antiwedge_bivec(u, v),      \
	                    TriVec: bivec_antiwedge_trivec(v, u)),     \
	Vec3  : _Generic(u, Vec3  : vec_antiwedge_vec(u, v)),
/* f ∨ g = Line where planes f and g intersect. Direction is zero if f and g are parallel. */
inline static BiVec trivec_antiwedge_trivec(TriVec f, TriVec g) {
	return (BiVec){
		.wx = f.z*g.y - f.y*g.z, /* e41 */
		.wy = f.x*g.z - f.z*g.x, /* e42 */
		.wz = f.y*g.x - f.x*g.y, /* e43 */
		.yz = f.x*g.w - f.w*g.x, /* e23 */
		.zx = f.y*g.w - f.w*g.y, /* e31 */
		.xy = f.z*g.w - f.w*g.z, /* e12 */
	};
}
/* L ∨ f = Point where line L intersects plane f. Weight is zero if L and f are parallel. */
static inline TriVec bivec_antiwedge_trivec(BiVec L, TriVec f) {
	return (TriVec){
		.x =  L.my*f.z - L.mz*f.y + L.vx*f.w, /* e1 */
		.y =  L.mz*f.x - L.mx*f.z + L.vy*f.w, /* e2 */
		.z =  L.mx*f.y - L.my*f.x + L.vz*f.w, /* e3 */
		.w = -L.vx*f.x - L.vy*f.y - L.vz*f.z, /* e4 */
	};
}
static inline float bivec_antiwedge_bivec(BiVec L1, BiVec L2) {
	return -antiwedge(L1.v, L2.m) - antiwedge(L2.v, L1.m);
}

/* -------------------------------------------------------------------- */

/* vec_triple(v, u, w) -> v ∧ u ∧ w */
// #define triple(v, u, w) _Generic(v, \
// 	Vec: _Generic(u, Vec: _Generic(w, Vec: vec4_triple(v, u, w)))
// inline static TriVec vec3_triple(Vec3 v, Vec3 u, Vec3 w) {
// 	return (TriVec){
// 		v.x*u.y*w.z + v.y*u.z*w.x + v.z*u.x*w.y -
// 		v.x*u.z*w.y - v.y*u.x*w.z - v.z*u.y*w.x
// 	};
// }

#define distance(a, b) _Generic(a,                   \
	Vec  : _Generic(b, Vec  : vec_vec_distance(a, b), \
	BiVec: _Generic(b, BiVec: bivec_bivec_distance(a, b)))
inline static float vec_vec_distance(Vec3 a, Vec3 b) {
	return sqrtf((a.x - b.x)*(a.x - b.x) + 
	             (a.y - b.y)*(a.y - b.y) +
	             (a.z - b.z)*(a.z - b.z));
}
/* Signed distance between two lines
 * d = (L1 ∨ L2)/(||v1 ∧ v2||)
 */
inline static float bivec_bivec_distance(BiVec L1, BiVec L2) {
	return antiwedge(L1, L2) / magnitude(wedge(L1.v, L2.v));
}
/* Signed distance between a point and a plane
 * d = (p ∨ f)/(||n||)
 */
inline static float vec_trivec_distance(Vec3 p, TriVec f) {
	return antiwedge(p, f) / magnitude(f.n);
}

#endif

/* ⎣f◍ ∧ p = Line perpendicular to plane f and passing through point p.
 *   ⎣f◍ = Normal vector of plane f (Weight left complement of f.)
 */
// static inline BiVec trivec_wedge_vec(TriVec f, Vec4 p) { // !!!
// 	return (BiVec){
// 		.wx = -f.x*p.w,          /* e41 */
// 		.wy = -f.y*p.w,          /* e42 */
// 		.wz = -f.z*p.w,          /* e43 */
// 		.yz = f.y*p.z - f.z*p.y, /* e23 */
// 		.zx = f.z*p.x - f.x*p.z, /* e31 */
// 		.xy = f.x*p.y - f.y*p.x, /* e12 */
// 	};
// }
/* ⎣L◍ ∧ p = Plane perpendicular to line L and containing point p.
 *   ⎣L◍ = Line at infinity perpendicular to line L. (Weight left complement of f.)
 */
// static inline TriVec bivec_wedge_vec(BiVec L, Vec4 p) { // !!!
// 	return (TriVec){
// 		.wyz = -L.vx*p.w,                      /* e423 */
// 		.wzx = -L.vy*p.w,                      /* e431 */
// 		.wxy = -L.vz*p.w,                      /* e412 */
// 		.zyx = L.vx*p.x + L.vy*p.y + L.vz*p.z, /* e321 */
// 	};
// }
/* ⎣f◍ ∧ L = Plane perpendicular to plane f and containing line L. Zero if L is perpendicular to f.
 *   ⎣f◍ = Normal vector of plane f (Weight left complement of f.)
 */
// static inline TriVec trivec_wedge_bivec(TriVec f, BiVec L) {
// 	return (TriVec){
// 		.wyz =  L.vy*f.z - L.vz*f.y,            /* e423 */
// 		.wzx =  L.vz*f.x - L.vx*f.z,            /* e431 */
// 		.wxy =  L.vx*f.y - L.vy*f.x,            /* e412 */
// 		.zyx = -L.mx*f.x - L.my*f.y - L.mz*f.z, /* e321 */
// 	};
// }
