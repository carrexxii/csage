#ifndef MATHS_QUATERNION_H
#define MATHS_QUATERNION_H

/* TODO: inlines */

#define QUAT_IDENT VEC4(0.0f, 0.0f, 0.0f, 1.0f)

void quat_print(vec4 q); /* "[Quaternion(mag|deg): x|y|z|w]" */

vec4 quat_new(float x, float y, float z, float w); /* q = (x, y, z, w) */
vec4 quat_new_vec(vec3 v, float w);               /* q = (v, w)       */

float quat_angle(vec4 q);              /* 2*acos(w)    */
float quat_mag(vec4 q);                /* ||q||        */
vec4  quat_normalise(vec4 q);          /* ||q|| ~= 1.0 */
void  quat_normalise_ip(vec4* q);
vec4  quat_conjugate(vec4 q);          /* -> q*        */
vec4  quat_mul(vec4 q, vec4 p);        /* p = q * p */
void  quat_mul_ip(vec4* q, vec4 p);
vec3  quat_rotate(vec4 q, vec3 v);     /* v' = q* * p(v) * q  */
void  quat_rotate_ip(vec4 q, vec3* v);

vec3 quat_to_vector(vec4 q);          /* truncates w (q[3]) component */
void quat_to_matrix(vec4 q, mat4* m); /* creates a rotation matrix    */

#endif
