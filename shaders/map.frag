#version 460

#include "common.glsl"

layout(location = 0) in vec3 Fpos;
layout(location = 1) in vec3 Fnormal;
layout(location = 2) in vec2 Fuv;

layout(location = 0) out vec4 Foutput;

layout(binding = 20) uniform sampler   Fsampler;
layout(binding = 21) uniform texture2D Falbedo;
layout(binding = 22) uniform texture2D Fnormals;

layout(binding = 1) uniform GlobalLightBufferUBO {
	DirectionalLight light;
} global;
layout(binding = 2) uniform LightBufferUBO {
	int point_lightc;
	int spot_lightc;
	PointLight point_lights[MAX_POINT_LIGHTS];
	SpotLight  spot_lights[MAX_SPOT_LIGHTS];
} local;

void main()
{
	vec4 albedo = texture(sampler2D(Falbedo, Fsampler), Fuv);
	vec3 normal = normalize(texture(sampler2D(Fnormals, Fsampler), Fuv).rgb * 2.0f - 1.0f);

	vec3 view_dir = normalize(-Fpos);

	vec3 total = calc_dir_light(global.light, normal, view_dir, albedo.rgb);
	for (int i = 0; i < local.spot_lightc; i++)
		total += calc_spot_light(local.spot_lights[i], normal, Fpos, view_dir, albedo.rgb);
	for (int i = 0; i < local.point_lightc; i++)
		total += calc_point_light(local.point_lights[i], normal, Fpos, view_dir, albedo.rgb);

	Foutput = vec4(total, albedo.a);
}
