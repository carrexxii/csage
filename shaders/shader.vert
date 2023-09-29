#version 460

layout(location = 0) in vec2 Vxy;
layout(location = 1) in vec3 Vrgb;

layout(location = 0) out vec3 Frgb;

layout(binding = 0) uniform UniformBufferObject {
	mat4 vp;
} cam;

void main()
{
	gl_Position = cam.vp * vec4(Vxy.x, -Vxy.y, 0.0, 1.0);
	Frgb = Vrgb;
}
