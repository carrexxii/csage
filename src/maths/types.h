#ifndef MATHS_TYPES_H
#define MATHS_TYPES_H

#include <inttypes.h>

#define RECT0 (Rect){ 0 }
#define RECT1 (Rect){ 0, 0, 1, 1 }

#define VEC2(x, y)       (Vec2){ x, y       }
#define VEC3(x, y, z)    (Vec3){ x, y, z    }
#define VEC4(x, y, z, w) (Vec4){ x, y, z, w }

#define VEC2I(x, y)       (Vec2i){ x, y       }
#define VEC3I(x, y, z)    (Vec3i){ x, y, z    }
#define VEC4I(x, y, z, w) (Vec4i){ x, y, z, w }

#define VEC2_V3(v)  (Vec2){ v.x    , v.y              }
#define VEC3_V2(v)  (Vec3){ v.x    , v.y    , 0.0f    }
#define VEC3_V3(v)  (Vec3){ v.x    , v.y    , v.z     }
#define VEC3_V4(v)  (Vec3){ v.x/v.w, v.y/v.w, v.z/v.w }
#define VEC3_A(arr) (Vec3){ arr[0] , arr[1] , arr[2]  }

#define VEC3I_V(v)   (Vec3i){ v.x    , v.y    , v.z     }
#define VEC3I_A(arr) (Vec3i){ arr[0] , arr[1] , arr[2]  }

#define VEC4_V3(v)  (Vec4){ v.x   , v.y   , v.z   , 0.0f    }
#define VEC4_V4(v)  (Vec4){ v.x   , v.y   , v.z   , v.w     }
#define VEC4_A(arr) (Vec4){ arr[0], arr[1], arr[2], arr[3]  }

#define VEC2_ZERO (Vec2){ .x = 0.0f, .y = 0.0f }
#define VEC3_ZERO (Vec3){ .x = 0.0f, .y = 0.0f, .z = 0.0f }
#define VEC3_ONE  (Vec3){ .x = 1.0f, .y = 1.0f, .z = 1.0f }
#define VEC3_X    (Vec3){ .x = 1.0f, .y = 0.0f, .z = 0.0f }
#define VEC3_Y    (Vec3){ .x = 0.0f, .y = 1.0f, .z = 0.0f }
#define VEC3_Z    (Vec3){ .x = 0.0f, .y = 0.0f, .z = 1.0f }
#define MAT4X4_IDENTITY (Mat4x4){ 1.0f, 0.0f, 0.0f, 0.0f, \
                                  0.0f, 1.0f, 0.0f, 0.0f, \
                                  0.0f, 0.0f, 1.0f, 0.0f, \
                                  0.0f, 0.0f, 0.0f, 1.0f, }

#define SCALAR(a)                     (float)(a)
#define VEC(x, y, z, w)               (Vec){ x, y, z, w }
#define DVEC(v)                       _Generic(v, Vec3: (Vec){ v.x, v.y, v.z, 0.0f })
#define PVEC(v)                       _Generic(v, Vec3: (Vec){ v.x, v.y, v.z, 1.0f })
#define BIVEC(E1, E2, E3, e1, e2, e3) (Bivec){ E1, E2, E3, e1, e2, e3 }
#define BIVECV(v1, v2)                (Bivec){ v1.x, v1.y, v1.z, v2.x, v2.y, v2.z }
#define TRIVEC(x, y, z, w)            (Trivec){ x, y, z, w }
#define PSS(a)                        (PsS){ .e0123 = (a) }

/* -------------------------------------------------------------------- */

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

typedef enum AxisMask {
	AXIS_NONE,
	AXIS_X = 1 << 0,
	AXIS_Y = 1 << 1,
	AXIS_Z = 1 << 2,
} AxisMask;

typedef enum DirectionMask {
	DIR_NONE         = 0,
	DIR_UP           = 1 << 0,
	DIR_DOWN         = 1 << 1,
	DIR_RIGHT        = 1 << 2,
	DIR_LEFT         = 1 << 3,
	DIR_FORWARDS     = 1 << 4,
	DIR_BACKWARDS    = 1 << 5,
	DIR_ROTATE_LEFT  = 1 << 6,
	DIR_ROTATE_RIGHT = 1 << 7,
	DIR_N            = 1 << 8,
	DIR_S            = 1 << 9,
	DIR_E            = 1 << 10,
	DIR_W            = 1 << 11,
	DIR_NW           = DIR_N | DIR_W,
	DIR_NE           = DIR_N | DIR_E,
	DIR_SW           = DIR_S | DIR_W,
	DIR_SE           = DIR_S | DIR_E,
	DIR_ALL          = DIR_N | DIR_S | DIR_E | DIR_W,
} DirectionMask;

typedef union Colour {
	uint32_t rgba;
	struct { uint8_t r, g, b, a; };
} Colour;

typedef union Point {
	struct { float  x,  y,  z; };
	struct { float E1, E2, E3; };
	float arr[3];
} Point;

typedef struct Rect {
	float x, y, w, h;
} Rect;
typedef struct Recti {
	int16_t x, y, w, h;
} Recti;

typedef struct Transform {
	Vec4  rot;
	Vec3  trans;
	float scale;
} Transform;

/* P(t) = p + t*v */
typedef struct Ray {
	Vec3 p;
	Vec3 v;
} Ray;

typedef struct Capsule {
	Vec3  p1, p2;
	float r;
} Capsule;

#endif

