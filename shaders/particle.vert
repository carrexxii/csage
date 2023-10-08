#version 460

layout(location = 0) in vec2 Vxy;

struct ParticleData {
	vec2 s, v;
	int  life;
};

layout(binding = 0) uniform CameraViewProjectionUBO {
	mat4 vp;
} camera;
layout(binding = 1) uniform ParticleDataUBO {
	ParticleData data[256];
} particles;

void main()
{	
	if (particles.data[gl_InstanceIndex].life <= 0) {
		gl_Position = vec4(2.0, 2.0, 0.0, 1.0);
	} else {
		vec2 s = particles.data[gl_InstanceIndex].s;
		gl_Position = camera.vp * vec4(Vxy.xy + s, 0.0, 1.0);
		gl_Position.y = -gl_Position.y;
	}
}
