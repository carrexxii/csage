#version 460

layout(location = 0) in vec3 Fpos;
layout(location = 1) in vec3 Fnormal;

layout(location = 0) out vec4 Foutput;

layout (binding = 20) uniform sampler   Fsampler;
layout (binding = 21) uniform texture2D Ftexture;

void main()
{
	Foutput = vec4(Fnormal, 1.0f);
}
