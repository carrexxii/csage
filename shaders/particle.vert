#version 460

layout(location = 0) in vec2 Vxy;
layout(location = 1) in vec2 Vuv;

layout(location = 0) out vec2 Fuv;

const int MAX_PARTICLES = 256;

struct ParticleData {
	vec2 s, v;
	int  life;
};

layout(set = 0, binding = 1) uniform CameraViewProjectionUBO {
	mat4 vp;
} camera;
layout(set = 0, binding = 2) uniform ParticleDataUBO {
	ParticleData data[MAX_PARTICLES];
	float scale;
} particles;

layout(push_constant) uniform PushConstants {
	float scale;
} constants;

void main()
{	
	if (particles.data[gl_InstanceIndex].life <= 0) {
		gl_Position = vec4(100.0, 100.0, 0.0, 1.0);
	} else {
		Fuv = Vuv;

		float scale = constants.scale;
		vec2 s = particles.data[gl_InstanceIndex].s;
		gl_Position = camera.vp * vec4(Vxy.xy*scale + s, -1.0/gl_InstanceIndex, 1.0);
		gl_Position.y = -gl_Position.y;
	}
}
