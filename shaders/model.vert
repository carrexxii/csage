#version 460

layout(location = 0) in vec3  Vpos;
layout(location = 1) in vec3  Vnormal;
layout(location = 2) in vec2  Vuv;
layout(location = 3) in uvec4 Vids;
layout(location = 4) in vec4  Vweights;

layout(location = 0) out vec3 Fpos;
layout(location = 1) out vec3 Fnormal;
layout(location = 2) out vec2 Fuv;

const int MAX_JOINTS = 32;

struct TransformData {
	vec4  rot;
	vec3  trans;
	float scale;
};
struct JointData {
	mat4 tform;
	mat4 inv_bind;
	int  parent;
};

layout(binding = 10) readonly buffer ObjectBufferSBO { mat4 objs[]; } mdls;
layout(binding = 11) readonly buffer SkinBufferSBO {
	float pad0[6];
	int jointc;
	JointData joints[];
} skin;

layout(binding = 0) uniform CameraBufferUBO { mat4 vp; } cam;
layout(binding = 3) uniform AnimationBufferUBO {
	TransformData kf1[MAX_JOINTS];
	TransformData kf2[MAX_JOINTS];
} anim;

layout(push_constant) uniform PushConstants {
	int   materiali;
	float timer;
} consts;

void build_transform_mat(vec4 q, vec3 p, out mat4 mat)
{
	// q -= normalize(vec4(0.04, 0.00, -0.00, -1.00));
	mat[0][0] = 1.0 - 2.0*q.y*q.y - 2.0*q.z*q.z;
	mat[0][1] =       2.0*q.x*q.y - 2.0*q.z*q.w;
	mat[0][2] =       2.0*q.x*q.z + 2.0*q.y*q.w;
	mat[0][3] = 0.0;//p.x;

	mat[1][0] =       2.0*q.x*q.y + 2.0*q.z*q.w;
	mat[1][1] = 1.0 - 2.0*q.x*q.x - 2.0*q.z*q.z;
	mat[1][2] =       2.0*q.y*q.z - 2.0*q.x*q.w;
	mat[1][3] = 0.0;//p.y;

	mat[2][0] =       2.0*q.x*q.z - 2.0*q.y*q.w;
	mat[2][1] =       2.0*q.y*q.z + 2.0*q.x*q.w;
	mat[2][2] = 1.0 - 2.0*q.x*q.x - 2.0*q.y*q.y;
	mat[2][3] = 0.0;//p.z;

	mat[3][0] = 0.0;
	mat[3][1] = 0.0;
	mat[3][2] = 0.0;
	mat[3][3] = 1.0;
}

vec4 apply_joint_transform(vec4 v)
{
	mat4 pose_mats[4];
	build_transform_mat(anim.kf1[Vids[0]].rot, anim.kf1[Vids[0]].trans, pose_mats[0]);
	build_transform_mat(anim.kf1[Vids[1]].rot, anim.kf1[Vids[1]].trans, pose_mats[1]);
	build_transform_mat(anim.kf1[Vids[2]].rot, anim.kf1[Vids[2]].trans, pose_mats[2]);
	build_transform_mat(anim.kf1[Vids[3]].rot, anim.kf1[Vids[3]].trans, pose_mats[3]);

	mat4 bone_trans = Vweights[0]*pose_mats[0] +
	                  Vweights[1]*pose_mats[1] +
	                  Vweights[2]*pose_mats[2] +
	                  Vweights[3]*pose_mats[3];

	return bone_trans * v;
}

void main()
{
	mat4 mdl = mdls.objs[gl_BaseInstance];

	vec4 final_pos;
	vec4 final_normal;

	final_pos    = apply_joint_transform(vec4(Vpos, 1.0));
	final_normal = apply_joint_transform(vec4(Vnormal, 0.0));

	Fpos    = vec3(mdl * final_pos);
	Fnormal = normalize(vec3(inverse(transpose(mdl)) * final_normal));
	Fuv     = Vuv;

	gl_Position = cam.vp * mdl * final_pos;
}
