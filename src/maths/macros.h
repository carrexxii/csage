#ifndef MATHS_MACROS_H
#define MATHS_MACROS_H

static inline noreturn void not_implemented(void) { assert(false); }

#define equal(a, b) _Generic(a,   \
		Vec2 : vec2_equal_vec2,   \
		Vec3 : vec3_equal_vec3,   \
		Vec4 : vec4_equal_vec4,   \
		Vec2i: vec2i_equal_vec2i, \
		Vec3i: vec3i_equal_vec3i, \
		Vec4i: vec4i_equal_vec4i  \
	)(a, b)

/*** reverse: a -> ~a ***/
/* ~a = (-1)^{x(x-1)/2}    (Note: only affects bivecs and trivecs in 3D PGA) */
#define reverse(a) _Generic(a,                                              \
		Vec  : reverse_vec   , Bivec: reverse_bivec, Trivec: reverse_trivec, \
		float: reverse_scalar, PsS  : reverse_pss                             \
	)(a)

/*** dual: a -> a* ***/
#define dual(a) _Generic(a,                                                 \
		Vec  : dual_of_vec   , Bivec: dual_of_bivec, Trivec: dual_of_trivec, \
		float: dual_of_scalar, PsS  : dual_of_pss                             \
	)(a)

/*** hodge: a -> ⋆a     (Note: only differs for trivec) ***/
/* ⋆S = (e0)(S)(I3)(-1)^(s(s+1)/2)
 *    = e0(S)e123 (* -1 for blades 1 and 2)
 */
#define hodge(a) _Generic(a,                                                   \
		Vec  : hodge_of_vec   , Bivec: hodge_of_bivec, Trivec: hodge_of_trivec, \
		float: hodge_of_scalar, PsS  : hodge_of_pss                              \
	)(a)

/*** add: a, b -> a + b ***/
#define add(a, b) _Generic((a),                         \
		float: _Generic(b, float : scalar_add_scalar,   \
		                   Vec   : scalar_add_vec,      \
		                   Vec2  : scalar_add_vec2,     \
		                   Vec3  : scalar_add_vec3,     \
		                   Vec4  : scalar_add_vec4,     \
		                   Vec2i : scalar_add_vec2i,    \
		                   Vec3i : scalar_add_vec3i,    \
		                   Vec4i : scalar_add_vec4i,    \
		                   Bivec : scalar_add_bivec,    \
		                   Trivec: scalar_add_trivec,   \
		                   PsS   : scalar_add_pss),     \
		Vec2: _Generic(b, float: vec2_add_scalar,       \
		                  Vec2 : vec2_add_vec2,         \
		                  Vec3 : not_implemented),      \
		Vec3: _Generic(b, float: vec3_add_scalar,       \
		                  Vec2 : not_implemented,       \
		                  Vec3 : vec3_add_vec3),        \
		Vec4: _Generic(b, float: vec4_add_scalar,       \
		                  Vec2 : not_implemented,       \
		                  Vec3 : not_implemented,       \
		                  Vec4 : vec4_add_vec4),        \
		Vec2i: _Generic(b, float: vec2i_add_scalar,     \
		                   Vec2 : not_implemented,      \
		                   Vec2i: vec2i_add_vec2i,      \
		                   Vec3 : not_implemented),     \
		Vec3i: _Generic(b, float: vec3i_add_scalar,     \
		                   Vec2 : not_implemented,      \
		                   Vec3 : not_implemented,      \
		                   Vec3i: vec3i_add_vec3i),     \
		Vec4i: _Generic(b, float: vec4i_add_scalar,     \
		                   Vec2 : not_implemented,      \
		                   Vec3 : not_implemented,      \
		                   Vec4i: vec4i_add_vec4i),     \
		Vec: _Generic(b, float: vec_add_scalar,         \
		                 Vec2 : not_implemented,        \
		                 Vec3 : not_implemented,        \
		                 Vec  : vec_add_vec),           \
		Bivec: _Generic(b, float: bivec_add_scalar,     \
		                   Vec2 : not_implemented,      \
		                   Vec3 : not_implemented,      \
		                   Bivec: bivec_add_bivec),     \
		Trivec: _Generic(b, float : trivec_add_scalar,  \
		                    Vec2  : not_implemented,    \
		                    Vec3  : not_implemented,    \
		                    Trivec: trivec_add_trivec), \
		PsS: _Generic(b, float: pss_add_scalar,         \
		                 Vec2 : not_implemented,        \
		                 Vec3 : not_implemented,        \
		                 PsS  : pss_add_pss)            \
	)((a), (b))

#define sub(a, b) _Generic(a, \
		Vec2: vec2_sub_vec2,  \
		Vec3: vec3_sub_vec3,  \
		Vec4: vec4_sub_vec4   \
	)(a, b)

/*** multiply: a, b -> a * b ***/
#define multiply(a, b) _Generic(a,                           \
		float: _Generic(b, float : scalar_multiply_scalar,   \
		                   Vec2  : scalar_multiply_vec2,     \
		                   Vec3  : scalar_multiply_vec3,     \
		                   Vec4  : scalar_multiply_vec4,     \
		                   Mat4x4: scalar_multiply_mat4x4,   \
		                   Vec   : scalar_multiply_vec,      \
		                   Bivec : scalar_multiply_bivec,    \
		                   Trivec: scalar_multiply_trivec,   \
		                   PsS   : scalar_multiply_pss),     \
		Vec2: _Generic(b, float : vec2_multiply_scalar,      \
		                  Mat4x4: not_implemented),          \
		Vec3: _Generic(b, float : vec3_multiply_scalar,      \
		                  Mat4x4: not_implemented),          \
		Vec4: _Generic(b, float : vec4_multiply_scalar,      \
		                  Mat4x4: vec4_multiply_mat4x4),     \
		Mat4x4: _Generic(b, float : mat4x4_multiply_scalar,  \
		                    Vec4  : mat4x4_multiply_vec4,    \
		                    Mat4x4: mat4x4_multiply_mat4x4), \
		Vec: _Generic(b, float : vec_multiply_scalar,        \
		                 Vec   : vec_multiply_vec,           \
		                 Mat4x4: not_implemented),           \
		Bivec: _Generic(b, float : bivec_multiply_scalar,    \
		                   Bivec : bivec_multiply_bivec,     \
		                   Mat4x4: not_implemented),         \
		Trivec: _Generic(b, float : trivec_multiply_scalar,  \
		                    Trivec: trivec_multiply_trivec,  \
		                    Mat4x4: not_implemented),        \
		PsS: _Generic(b, float : pss_multiply_scalar,        \
		                 PsS   : pss_multiply_pss,           \
		                 Mat4x4: not_implemented)            \
	)(a, b)

/*** wedge: a, b -> a ∧ b ***/
#define wedge(a, b) _Generic(a,                           \
		float: _Generic(b, float: scalar_wedge_scalar,    \
		                     Vec   : scalar_wedge_vec,    \
		                     Bivec : scalar_wedge_bivec,  \
		                     Trivec: scalar_wedge_trivec, \
		                     PsS   : scalar_wedge_pss),   \
		Vec: _Generic(b, float: vec_wedge_scalar,         \
		                  Vec   : vec_wedge_vec,          \
		                  Bivec : vec_wedge_bivec,        \
		                  Trivec: vec_wedge_trivec),      \
		Bivec: _Generic(b, float: bivec_wedge_scalar,     \
		                    Vec   : bivec_wedge_vec,      \
		                    Bivec : bivec_wedge_bivec)    \
	)(a, b)

/* inner: a, b -> a⋅b */
#define inner(a, b) _Generic(a,                           \
		float: _Generic(b, float: scalar_inner_scalar,    \
		                    Vec   : scalar_inner_vec,     \
		                    Bivec : scalar_inner_bivec,   \
		                    Trivec: scalar_inner_trivec), \
		Vec: _Generic(b, float: vec_inner_scalar,         \
		                 Vec   : vec_inner_vec,           \
		                 Bivec : vec_inner_bivec,         \
		                 Trivec: vec_inner_trivec),       \
		Bivec: _Generic(b, float: bivec_inner_scalar,     \
		                   Vec   : bivec_inner_vec,       \
		                   Bivec : bivec_inner_bivec,     \
		                   Trivec: bivec_inner_trivec),   \
		Trivec: _Generic(b, float: trivec_inner_scalar,   \
		                    Vec   : trivec_inner_vec,     \
		                    Bivec : trivec_inner_bivec,   \
		                    Trivec: trivec_inner_trivec)  \
	)(a, b)

/*** norm2: a -> ||a||^2 ***/
/* ||a||^2 = <(a)(~A)>0 */
#define norm2(a) _Generic(a,         \
		float: scalar_inner_scalar,  \
		Vec   : vec_inner_vec,       \
		Vec3  : vec3_inner_vec3,     \
		Bivec : bivec_inner_bivec,   \
		Trivec: trivec_inner_trivec, \
		PsS   : pss_inner_pss        \
	)(a, a)

/*** norm: a -> sqrt(norm2(a)) ***/
#define norm(a) _Generic(a,  \
		float : scalar_norm, \
		Vec   : vec_norm,    \
		Vec3  : vec3_norm,   \
		Bivec : bivec_norm,  \
		Trivec: trivec_norm, \
		PsS   : pss_norm     \
	)(a)

/*** normalized: a -> a / ||a|| ***/
#define normalized(a) _Generic(a,  \
		Vec   : vec_normalized,    \
		Vec2  : vec2_normalized,   \
		Vec3  : vec3_normalized,   \
		Vec4  : vec4_normalized,   \
		Bivec : bivec_normalized,  \
		Trivec: trivec_normalized  \
	)(a)

/*** join: a, b -> a ∨ b ***/
/* a ∨ b = ⋆-(⋆a ∧ ⋆b) */
/* ⋆-S = ⋆S(-1)^(grade(S)d) = ⋆S (* -1 for blades 1 and 3)*/
#define join(a, b) _Generic(a,                         \
		Bivec : _Generic(b, Trivec: bivec_join_trivec), \
		Trivec: _Generic(b, Trivec: trivec_join_trivec)  \
	)(a, b)

#define is_zero(a) _Generic(a, \
		Vec2: vec2_is_zero,    \
		Vec3: vec3_is_zero,    \
		Vec4: vec4_is_zero     \
	)(a)

#define print(a) _Generic(a,                                           \
		Vec2  : print_vec2  , Vec3 : print_vec3 , Vec4  : print_vec4  , \
		Mat4x4: print_mat4x4,                                            \
		Vec   : print_vec   , Bivec: print_bivec, Trivec: print_trivec,   \
		float : print_scalar, PsS  : print_pss                             \
	)(a)

/* -------------------------------------------------------------------- */

#define dot(a, b) _Generic(a, \
		Vec2: vec2_dot_vec2,  \
		Vec3: vec3_dot_vec3,  \
		Vec4: vec4_dot_vec4   \
	)(a, b)

#define distance2(a, b) _Generic(a, \
		Vec2: vec2_distance2_vec2,  \
		Vec3: vec3_distance2_vec3   \
	)(a, b)

#define distance(a, b) _Generic(a, \
		Vec2: vec2_distance_vec2,  \
		Vec3: vec3_distance_vec3   \
	)(a, b)

#define rotate(a, axis, angle) _Generic(a, \
		Vec3  : rotate_vec3,               \
		Mat4x4: rotate_mat4x4              \
	)(a, axis, angle)

#define inverse(a) _Generic(a, \
		Mat4x4: mat4x4_inverse \
	)(a)

/* -------------------------------------------------------------------- */

#define lerp(a, b, t) _Generic(a, \
		Vec3: vec3_lerp           \
	)(a, b, t)

#endif
