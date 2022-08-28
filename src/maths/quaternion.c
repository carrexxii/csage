#include "maths.h"
#include "maths/quaternion.h"

#if DEBUG_LEVEL > 0
	void quat_print(vec4 q)
	{
		double m = (double)quat_mag(q),
		       a = to_degd((double)quat_angle(q));
		printf("[Quaternion(%.3f|%.3f*): %.3f|%.3f|%.3f|%.3f]\n", m, a, q.x, q.y, q.z, q.w);
	}
#else
	#define quat_print(q)
#endif

vec4 quat_new(float x, float y, float z, float w)
{
	float sina = sin(w / 2.0);

	return quat_normalise(VEC4(sina * x, sina * y, sina * z, cosf(w / 2.0)));
}

vec4 quat_new_vec(vec3 v, float w)
{
	return quat_new(v.x, v.y, v.z, w);
}

float quat_angle(vec4 q)
{
	return 2.0f * acos(q.w);
}

float quat_mag(vec4 q)
{
	return sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
}

vec4 quat_normalise(vec4 q)
{
	vec4 p = q;
	quat_normalise_ip(&p);

	return p;
}

void quat_normalise_ip(vec4* q)
{
	float mag = quat_mag(*q);
	if (!is_equal(mag, 1.0f)) {
		q->x /= mag;
		q->y /= mag;
		q->z /= mag;
		q->w /= mag;
	}
}

vec4 quat_conjugate(vec4 q)
{
	return VEC4(-q.x, -q.y, -q.z, q.w);
}

vec4 quat_mul(vec4 q, vec4 p)
{
	vec4 r = q;
	quat_mul_ip(&r, p);

	return r;
}

void quat_mul_ip(vec4* q, vec4 p)
{
	q->x = p.w*q->x + p.x*q->w + p.y*q->z - p.z*q->y;
	q->y = p.w*q->y + p.y*q->w + p.z*q->x - p.x*q->z;
	q->z = p.w*q->z + p.z*q->w + p.x*q->y - p.y*q->x;
	q->w = p.w*q->w - p.x*q->x - p.y*q->y - p.z*q->z;
	quat_normalise_ip(q);
}

/* v' = q* * p(v) * q */
vec3 quat_rotate(vec4 q, vec3 v)
{
	vec4 p  = quat_new_vec(v, 0.0);
	vec4 qc = quat_conjugate(q);
	vec4 r  = quat_mul(quat_mul(qc, p), p);

	return VEC3(r.x, r.y, r.z);
}

void quat_rotate_ip(vec4 q, vec3* v)
{
	*v = quat_rotate(q, *v);
}

vec3 quat_to_vector(vec4 q)
{
	if (is_equal(q.w, 1.0)) {
		return VEC3(1.0, 0.0, 0.0);
	} else {
		float sina = sin(acos(q.w));

		return VEC3(q.x / sina, q.y / sina, q.z / sina);
	}
}

void quat_to_matrix(vec4 q, mat4* m)
{
	float x = q.x,
	      y = q.y,
	      z = q.z,
	      w = q.w;

	m->m11  = 1.0 - 2.0*y*y - 2.0*z*z; // 1 - 2y^2 - 2z^2
	m->m12  =       2.0*x*y - 2.0*z*w; //     2xy  - 2zw
	m->m13  =       2.0*x*z + 2.0*y*w; //     2xz  + 2yw
	m->m14  = 0.0;

	m->m21 =       2.0*x*y + 2.0*z*w; //     2xy  + 2zw
	m->m22 = 1.0 - 2.0*x*x - 2.0*z*z; // 1 - 2x^2 - 2z^2
	m->m23 =       2.0*y*z - 2.0*x*w; //     2yz  - 2xw
	m->m24 = 0.0;

	m->m31 =       2.0*x*z - 2.0*y*w; //     2xz  - 2yw
	m->m32 =       2.0*y*z + 2.0*x*w; //     2yz  + 2xw
	m->m33 = 1.0 - 2.0*x*x - 2.0*y*y; // 1 - 2x^2 - 2y^2
	m->m34 = 0.0;

	m->m41 = 0.0;
	m->m42 = 0.0;
	m->m43 = 0.0;
	m->m44 = 1.0;
}
