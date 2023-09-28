#version 460

layout(location = 0) in vec2 Vxy;
layout(location = 1) in vec3 Vrgb;

layout(location = 0) out vec2 Fxy;
layout(location = 1) out vec3 Frgb;

layout(binding = 1) uniform UniformBufferObject {
	mat3 vp;
} cam;

void main()
{
	Fxy  = Vxy;
	Frgb = Vrgb;
}
