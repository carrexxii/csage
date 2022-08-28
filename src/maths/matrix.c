#include "maths.h"

const mat3 MAT_I3 = MAT3_IDENT;
const mat4 MAT_I4 = MAT4_IDENT;

#if DEBUG_LEVEL > 0
	void mat_print_(const float* a, int dim)
	{
		printf("Matrix%dx%d:\n", dim, dim);
		for (int j = 0; j < dim; j++) {
			fprintf(stderr, (j % (dim-1) == 0? "   [": "   |"));
			for (int i = 0; i < dim; i++) {
				fprintf(stderr, "%6.3f ", (double)a[j*dim + i]);
			}
			DEBUG(1, "\b%s", (j % (dim-1) == 0? " ]": " |"));
		}
	}
#else
	#define mat_print_(a, dim)
#endif

void mat4_rotate(mat4* a, enum Axis axis, float rad)
{
	mat4 rot = MAT4_IDENT;
	switch (axis) {
		case AXIS_NONE:
			ERROR("[MATHS] No Axis specified");
			return;
		case AXIS_X:
			rot.m22 =  cos(rad);
			rot.m23 = -sin(rad);
			rot.m32 = -rot.m23;
			rot.m33 =  rot.m22;
			break;
		case AXIS_Y:
			rot.m11 =  cos(rad);
			rot.m13 =  sin(rad);
			rot.m32 = -rot.m13;
			rot.m33 =  rot.m11;
			break;
		case AXIS_Z:
			rot.m11 =  cos(rad);
			rot.m12 = -sin(rad);
			rot.m21 = -rot.m12;
			rot.m22 =  rot.m11;
			break;
	}
	mat_mul_ip(a, &rot);
}

void mat_new_persp(mat4* mat, float ar, float fov, float zn, float zf)
{
	float f   = (1.0 / tan(0.5*fov)),
	      nmf = zn - zf;
	mat_copy(mat, &((mat4){ f/ar, 0.0,         0.0, 0.0,
	                         0.0,   f,         0.0, 0.0,
	                         0.0, 0.0,     -zf/nmf, 1.0,
	                         0.0, 0.0, (zn*zf)/nmf, 0.0, }));
}

void mat_new_ortho(mat4* mat, float t, float b, float r, float l, float zn, float zf)
{
	float rpl = r + l, rml = r - l,
	      bpt = b + t, bmt = b - t,
	      nmf = zn - zf;
	mat_copy(mat, &((mat4){ 2.0/rml,      0.0,      0.0, 0.0,
	                            0.0,  2.0/bmt,      0.0, 0.0,
	                            0.0,      0.0, -1.0/nmf, 0.0,
	                       -rpl/rml, -bpt/bmt,  -zn/nmf, 1.0, }));
}

void mat_transpose(mat4* a)
{
	mat4 b;
	mat_copy(&b, a);
	a->m12 = b.m21;
	a->m13 = b.m31;
	a->m14 = b.m41;
	a->m21 = b.m12;
	a->m23 = b.m32;
	a->m24 = b.m42;
	a->m31 = b.m13;
	a->m32 = b.m23;
	a->m34 = b.m43;
	a->m41 = b.m14;
	a->m42 = b.m24;
	a->m43 = b.m34;
}

void mat_unproject(vec4* v, mat4* inv, vec4 vp)
{
	v->x = (2.0*v->x - vp.i)/vp.k - 0.5;
	v->y = (2.0*v->y - vp.j)/vp.l - 0.5;
	v->z = 0.0;
	v->w = 1.0;
	// vec_print(v);
	mat_mul_v_ip(v, inv);

	// float tmp = v->x;
	// v->x = v->y;
	// v->y = -tmp;
}

void mat4_transform(mat4* a, struct MatTrans* ts, int tc)
{
	mat4 trans = MAT4_IDENT;
	for (int i = 0; i < tc; i++) {
		switch (ts[i].type) {
		case MTRANS_TRANSLATE: mat_translate(&trans, ts[i].v)                ; break;
		case MTRANS_SCALE    : mat_scale_v(  &trans, ts[i].v)                ; break;
		case MTRANS_ROTATE   : mat_rotate(   &trans, ts[i].axis, ts[i].angle); break;
		default:
			ERROR("[MATHS] %d is not a supported transform", ts[i].type);
			continue;
		}
	}
	mat_mul_ip(a, &trans);
}

/* modified from: gluInvertMatrix */
void mat4_inv(mat4* a)
{
	mat4 inv;
	inv.m11 =  a->m22*a->m33*a->m44 - a->m22*a->m34*a->m43 -
	           a->m32*a->m23*a->m44 + a->m32*a->m24*a->m43 +
	           a->m42*a->m23*a->m34 - a->m42*a->m24*a->m33;
	inv.m21 = -a->m21*a->m33*a->m44 + a->m21*a->m34*a->m43 +
	           a->m31*a->m23*a->m44 - a->m31*a->m24*a->m43 -
	           a->m41*a->m23*a->m34 + a->m41*a->m24*a->m33;
	inv.m31 =  a->m21*a->m32*a->m44 - a->m21*a->m34*a->m42 -
	           a->m31*a->m22*a->m44 + a->m31*a->m24*a->m42 +
	           a->m41*a->m22*a->m34 - a->m41*a->m24*a->m32;
	inv.m41 = -a->m21*a->m32*a->m43 + a->m21*a->m33*a->m42 +
	           a->m31*a->m22*a->m43 - a->m31*a->m23*a->m42 -
	           a->m41*a->m22*a->m33 + a->m41*a->m23*a->m32;
	inv.m12 = -a->m12*a->m33*a->m44 + a->m12*a->m34*a->m43 +
	           a->m32*a->m13*a->m44 - a->m32*a->m14*a->m43 -
	           a->m42*a->m13*a->m34 + a->m42*a->m14*a->m33;
	inv.m22 =  a->m11*a->m33*a->m44 - a->m11*a->m34*a->m43 -
	           a->m31*a->m13*a->m44 + a->m31*a->m14*a->m43 +
	           a->m41*a->m13*a->m34 - a->m41*a->m14*a->m33;
	inv.m32 = -a->m11*a->m32*a->m44 + a->m11*a->m34*a->m42 +
	           a->m31*a->m12*a->m44 - a->m31*a->m14*a->m42 -
	           a->m41*a->m12*a->m34 + a->m41*a->m14*a->m32;
	inv.m42 =  a->m11*a->m32*a->m43 - a->m11*a->m33*a->m42 -
	           a->m31*a->m12*a->m43 + a->m31*a->m13*a->m42 +
	           a->m41*a->m12*a->m33 - a->m41*a->m13*a->m32;
	inv.m13 =  a->m12*a->m23*a->m44 - a->m12*a->m24*a->m43 -
	           a->m22*a->m13*a->m44 + a->m22*a->m14*a->m43 +
	           a->m42*a->m13*a->m24 - a->m42*a->m14*a->m23;
	inv.m23 = -a->m11*a->m23*a->m44 + a->m11*a->m24*a->m43 +
	           a->m21*a->m13*a->m44 - a->m21*a->m14*a->m43 -
	           a->m41*a->m13*a->m24 + a->m41*a->m14*a->m23;
	inv.m33 =  a->m11*a->m22*a->m44 - a->m11*a->m24*a->m42 -
	           a->m21*a->m12*a->m44 + a->m21*a->m14*a->m42 +
	           a->m41*a->m12*a->m24 - a->m41*a->m14*a->m22;
	inv.m43 = -a->m11*a->m22*a->m43 + a->m11*a->m23*a->m42 +
	           a->m21*a->m12*a->m43 - a->m21*a->m13*a->m42 -
	           a->m41*a->m12*a->m23 + a->m41*a->m13*a->m22;
	inv.m14 = -a->m12*a->m23*a->m34 + a->m12*a->m24*a->m33 +
	           a->m22*a->m13*a->m34 - a->m22*a->m14*a->m33 -
	           a->m32*a->m13*a->m24 + a->m32*a->m14*a->m23;
	inv.m24 =  a->m11*a->m23*a->m34 - a->m11*a->m24*a->m33 -
	           a->m21*a->m13*a->m34 + a->m21*a->m14*a->m33 +
	           a->m31*a->m13*a->m24 - a->m31*a->m14*a->m23;
	inv.m34 = -a->m11*a->m22*a->m34 + a->m11*a->m24*a->m32 +
	           a->m21*a->m12*a->m34 - a->m21*a->m14*a->m32 -
	           a->m31*a->m12*a->m24 + a->m31*a->m14*a->m22;
	inv.m44 =  a->m11*a->m22*a->m33 - a->m11*a->m23*a->m32 -
	           a->m21*a->m12*a->m33 + a->m21*a->m13*a->m32 +
	           a->m31*a->m12*a->m23 - a->m31*a->m13*a->m22;

	float det = a->m11*inv.m11 + a->m12*inv.m21 + a->m13*inv.m31 + a->m14*inv.m41;
	if (fabs(det) <= FLT_EPSILON)
	    ERROR("[MATHS] matrix is not invertible");

	det = 1.0 / det;

	for (uint i = 0; i < 16; i++)
	    a->arr[i] = det * inv.arr[i];
}
