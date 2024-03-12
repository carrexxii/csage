#version 460

#include "common.glsl"

layout(location = 0) in vec3 Fpos;
layout(location = 1) in vec2 Fuv;

layout(location = 0) out vec4 Foutput;

layout(binding = 20) uniform sampler   Fsampler;
layout(binding = 21) uniform texture2D Falbedo;

layout(binding = 1) uniform GlobalLightBufferUBO {
	DirectionalLight light;
} global;

void main()
{
	vec4 albedo = texture(sampler2D(Falbedo, Fsampler), Fuv);

	Foutput = vec4(Fpos, albedo.a);
}
