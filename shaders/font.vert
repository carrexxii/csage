#version 460

layout(location = 0) in vec2 Vxy;
layout(location = 1) in vec2 Vuv;

layout(location = 0) out vec2 Fuv;

layout(binding = 1) uniform UniformBufferObject {
	mat4 vp;
} cam;

void main()
{
	Fuv = Vuv;

	gl_Position = vec4(Vxy, 0.0, 1.0);
	gl_Position.y = -gl_Position.y;
}
