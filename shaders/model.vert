#version 460

layout(location = 0) in vec3  Vpos;
layout(location = 1) in vec3  Vnormal;
layout(location = 2) in vec2  Vuv;
layout(location = 3) in uvec4 Vjoint_ids;
layout(location = 4) in vec4  Vjoint_weights;

layout(location = 0) out vec3 Fpos;
layout(location = 1) out vec3 Fnormal;
layout(location = 2) out vec2 Fuv;

struct ObjectData { mat4 mat; };
layout(set = 0, binding = 1) readonly buffer ObjectBufferSBO {
	ObjectData objs[];
} mdls;

layout(set = 0, binding = 2) uniform CameraBufferUBO {
	mat4 vp;
} cam;

const int MAX_BONES = 64;
struct AnimationData {
	vec4  rotation;
	vec3  translation;
	float scale;
};
layout(set = 0, binding = 5) uniform AnimationBufferUBO {
	AnimationData transforms[MAX_BONES];
} anim;

layout(push_constant) uniform PushConstants {
	int   materiali;
	float timer;
} constants;

void quat_to_mat(vec4 q, out mat4 mat)
{
	mat[0][0] = 1.0 - 2.0*q.y*q.y - 2.0*q.z*q.z;
	mat[0][1] =       2.0*q.x*q.y - 2.0*q.z*q.w;
	mat[0][2] =       2.0*q.x*q.z + 2.0*q.y*q.w;
	mat[0][3] = 0.0;

	mat[1][0] =       2.0*q.x*q.y + 2.0*q.z*q.w;
	mat[1][1] = 1.0 - 2.0*q.x*q.x - 2.0*q.z*q.z;
	mat[1][2] =       2.0*q.y*q.z - 2.0*q.x*q.w;
	mat[1][3] = 0.0;

	mat[2][0] =       2.0*q.x*q.z - 2.0*q.y*q.w;
	mat[2][1] =       2.0*q.y*q.z + 2.0*q.x*q.w;
	mat[2][2] = 1.0 - 2.0*q.x*q.x - 2.0*q.y*q.y;
	mat[2][3] = 0.0;

	mat[3][0] = 0.0;
	mat[3][1] = 0.0;
	mat[3][2] = 0.0;
	mat[3][3] = 1.0;
}

void main()
{
	mat4 mdl = mdls.objs[gl_BaseInstance].mat;

	vec3 final_pos;
	vec3 final_normal;

	mat4 rot_mats[4];
	quat_to_mat(anim.transforms[Vjoint_ids[0]].rotation, rot_mats[0]);
	quat_to_mat(anim.transforms[Vjoint_ids[1]].rotation, rot_mats[1]);
	quat_to_mat(anim.transforms[Vjoint_ids[2]].rotation, rot_mats[2]);
	quat_to_mat(anim.transforms[Vjoint_ids[3]].rotation, rot_mats[3]);

	mat4 bone_transform = rot_mats[0] * Vjoint_weights[0];
	bone_transform     += rot_mats[1] * Vjoint_weights[1];
	bone_transform     += rot_mats[2] * Vjoint_weights[2];
	bone_transform     += rot_mats[3] * Vjoint_weights[3];

	final_pos    = vec3(bone_transform * vec4(Vpos, 1.0));
	final_normal = vec3(bone_transform * vec4(Vnormal, 0.0));

	Fpos    = vec3(mdl * vec4(final_pos, 1.0));
	Fnormal = normalize(vec3(inverse(transpose(mdl)) * vec4(final_normal, 0.0)));
	Fuv     = Vuv;

	gl_Position = cam.vp * mdl * vec4(final_pos, 1.0);
}
