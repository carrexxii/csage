#version 460
#extension GL_EXT_nonuniform_qualifier: enable

#include "common.glsl"

layout(location = 0) in vec3 Fpos;
layout(location = 1) in vec2 Fuv;

layout(location = 0) out vec4 Foutput;

layout(binding = 1) uniform GlobalLightBufferUBO {
	DirectionalLight light;
} global;

layout(binding = 12) readonly buffer SpotLightDataSBO {
	int count;
	SpotLight data[];
} spot_lights;

layout(binding = 13) readonly buffer PointLightDataSBO {
	int count;
	PointLight data[];
} point_lights;

layout(binding = 20) uniform sampler   Fsampler;
layout(binding = 21) uniform texture2D Falbedo;
layout(binding = 22) uniform texture2D Fnormal;

layout(push_constant) uniform PushConstants {
	int sheet;
	mat4 view;
} push;

void main()
{
	vec4 albedo = texture(sampler2D(Falbedo, Fsampler), Fuv);
	if (albedo.w < 0.5) discard;
	vec3 normal = normalize(texture(sampler2D(Fnormal, Fsampler), Fuv).rgb * 2.0f - 1.0f);

	vec3 view_dir = normalize(-Fpos);

	vec3 total = calc_dir_light(global.light, normal, view_dir, albedo.rgb);
	for (int i = 0; i < spot_lights.count; i++)
		total += calc_spot_light(spot_lights.data[i], normal, Fpos, view_dir, albedo.rgb);
	for (int i = 0; i < point_lights.count; i++)
		total += calc_point_light(point_lights.data[i], normal, Fpos, view_dir, albedo.rgb);

	Foutput = vec4(total, albedo.a);
	// if (distance(Fpos, point_lights.data[0].pos) < 0.5)
		// Foutput = vec4(1, 0, 0, 1);
}
