#version 460

layout(location = 0) in vec3  Vpos;
layout(location = 1) in vec3  Vnormal;
layout(location = 2) in vec2  Vuv;
layout(location = 3) in ivec4 Vjoint_IDs;
layout(location = 4) in vec4  Vjoint_weights;

layout(location = 0) out vec2 Fuv;
layout(location = 1) out vec3 Fnormal;
layout(location = 2) out vec3 Fpos;

struct ObjectData { mat4 mat; };
layout(set = 0, binding = 1) readonly buffer ObjectBuffer {
	ObjectData objs[];
} mdls;

layout(set = 0, binding = 4) uniform CameraBuffer {
	mat4 vp;
} cam;

const int MAX_BONES = 64;
struct AnimationData {
	vec4 rotation;
	vec3 translation;
	vec3 scale;
};
layout(set = 0, binding = 5) uniform AnimationBuffer {
	AnimationData transforms[MAX_BONES];
} anim;

layout(push_constant) uniform PushConstants {
	int   materiali;
	float timer;
} constants;


void main()
{
	mat4 mdl = mdls.objs[gl_BaseInstance].mat;

	Fuv     = Vuv;
	Fnormal = normalize(vec3(mdl * vec4(Vnormal, 1.0)));
	Fpos    = vec3(mdl * vec4(Vpos, 1.0));

	// vec3 anim_pos = Vpos;
	// for (int i = 0; i < 4; i++) {
	// 	if (Vjoint_IDs[i] == -1)
	// 		continue;
	// 	else if (Vjoint_IDs[i] >= MAX_BONES)
	// 		break;
	// 	anim_pos += anim.transforms[Vjoint_IDs[i]].translation * Vjoint_weights[i];
	// }

	// gl_Position = cam.vp * mdl * vec4(anim_pos, 1.0);
	gl_Position = cam.vp * mdl * vec4(Vpos, 1.0);
}
