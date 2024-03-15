#version 460
#extension GL_EXT_nonuniform_qualifier: enable

#include "common.glsl"

layout(location = 0) in vec3 Fpos;
layout(location = 1) in vec2 Fuv;

layout(location = 0) out vec4 Foutput;

layout(binding = 20) uniform sampler   Fsampler;
layout(binding = 21) uniform texture2D Falbedo[];

layout(binding = 1) uniform GlobalLightBufferUBO {
	DirectionalLight light;
} global;

layout(push_constant) uniform PushConstants {
	int sheet;
} push;

void main()
{
	vec4 albedo = texture(sampler2D(Falbedo[push.sheet], Fsampler), Fuv);

	Foutput = vec4(albedo);
}
