#ifndef MATHS_MATHS_H
#define MATHS_MATHS_H

#include <tgmath.h>

#include <cblas.h>

#define PI      3.14159265358979323846264338327950288
#define PI_2    1.57079632679489661923132169163975144
#define PI_3    1.04719755119659774615421446109316763
#define PI_4    0.785398163397448309615660845819875721
#define PI_5    (PI/5.0)
#define PI_6    (PI_3/2.0)
#define SQRT2   1.41421356237309504880168872420969808
#define SQRT2_2 0.707106781186547524400844362104849039
#define E       2.71828182845904523536028747135266250

enum Axis {
	AXIS_NONE = 0x0,
	AXIS_X    = 0x01,
	AXIS_Y    = 0x02,
	AXIS_Z    = 0x04,
};

struct Rect {
	float x, y, w, h;
}; static_assert(sizeof(struct Rect) == 16, "struct Rect");

#define is_equal(_a, _b) (bool)(_Generic((_a), \
	float : (fabs((_a) - (_b)) < FLT_EPSILON),  \
	double: (fabs((_a) - (_b)) < DBL_EPSILON)))

inline static float  to_radf(float  deg) { return (deg / 360.0) * (2.0 * PI); }
inline static double to_radd(double deg) { return (deg / 360.0) * (2.0 * PI); }
inline static float  to_degf(float  rad) { return (rad / (2.0 * PI)) * 360.0; }
inline static double to_degd(double rad) { return (rad / (2.0 * PI)) * 360.0; }

#include "vector.h"
#include "matrix.h"
#include "quaternion.h"

#endif
