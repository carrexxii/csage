#ifndef MATHS_TYPES_H
#define MATHS_TYPES_H

typedef union Vec2 { struct { float x, y;       }; float arr[2]; } Vec2;
typedef union Vec3 { struct { float x, y, z;    }; float arr[3]; } Vec3;
typedef union Vec4 { struct { float x, y, z, w; }; float arr[4]; } Vec4;
typedef union Vec2i { struct { int x, y;       }; int arr[2]; } Vec2i;
typedef union Vec3i { struct { int x, y, z;    }; int arr[3]; } Vec3i;
typedef union Vec4i { struct { int x, y, z, w; }; int arr[4]; } Vec4i;
typedef union Mat4x4 {
	struct { Vec4 r1, r2, r3, r4; };
	struct { float m11, m12, m13, m14,
	               m21, m22, m23, m24,
	               m31, m32, m33, m34,
	               m41, m42, m43, m44; };
	float arr[16];
} Mat4x4;

/* -------------------------------------------------------------------- */

typedef union Vec {
	struct { float  x,  y,  z,  w; };
	struct { float e1, e2, e3, e0; };
	struct { Vec3 v; float h; }; // TODO: better name than h
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

/* -------------------------------------------------------------------- */

typedef union Point {
	struct { float  x,  y,  z; };
	struct { float E1, E2, E3; };
	float arr[3];
} Point;

typedef struct Rect {
	float x, y, w, h;
} Rect;
typedef struct Recti {
	int16 x, y, w, h;
} Recti;

struct Transform {
	Vec4 rot;
	Vec3 trans;
	float scale;
};

/* P(t) = p + t*v */
struct Ray {
	Vec3 p;
	Vec3 v;
};

struct Capsule {
	Vec3 p1, p2;
	float r;
};

#endif
