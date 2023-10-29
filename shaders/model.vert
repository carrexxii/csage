#version 460

layout(location = 0) in vec3  Vpos;
layout(location = 1) in vec3  Vnormal;
layout(location = 2) in vec2  Vuv;
layout(location = 3) in ivec4 Vjoint_IDs;
layout(location = 4) in vec4  Vjoint_weights;

layout(location = 0) out vec2 Fuv;
layout(location = 1) out vec3 Fnnn;

struct ObjectData { mat4 mat; };
layout(std140, binding = 0) readonly buffer ObjectBuffer {
	ObjectData objs[];
} mdls;

layout(binding = 1) uniform CameraBuffer {
	mat4 vp;
} cam;

struct AnimationData {
	vec4 rotation;
	vec3 translation;
	vec3 scale;
};
layout(binding = 2) uniform AnimationBuffer {
	AnimationData transforms[64];
} anim;

layout(push_constant) uniform PushConstants {
	int   materiali;
	float timer;
} constants;

void main()
{
	Fuv  = Vuv;
	Fnnn = Vnormal;
	Fnnn = vec3(constants.timer);
	
	for (int i = 0; i < 4; i++) {
		if (Vjoint_IDs[i] == -1)
			continue;
	}

	mat4 mdl = mdls.objs[gl_BaseInstance].mat;
	gl_Position = cam.vp * mdl * vec4(Vpos, 1.0);
}
