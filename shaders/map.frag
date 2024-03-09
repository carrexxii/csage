#version 460

#include "common.glsl"

layout(location = 0) in vec3 Fpos;
layout(location = 1) in vec2 Fuv;
layout(location = 2) in flat uint Fchunk_id;

layout(location = 0) out vec4 Foutput;

layout(binding = 20) uniform sampler   Fsampler;
layout(binding = 21) uniform texture2D Falbedo;
layout(binding = 22) uniform texture2D Fnormals;

layout(binding = 2) uniform MapDataUBO {
	ivec2 chunk_dim;
	ivec2 tex_dim;
} map;

layout(binding = 1) uniform GlobalLightBufferUBO {
	DirectionalLight light;
} global;

layout(binding = 10) readonly buffer ChunkDataSBO {
	MapChunkData data[];
} chunks;

layout(binding = 11) readonly buffer SpotLightDataSBO {
	SpotLight data[];
} spot_lights;

layout(binding = 12) readonly buffer PointLightDataSBO {
	PointLight data[];
} point_lights;

void main()
{
	vec4 albedo = texture(sampler2D(Falbedo, Fsampler), Fuv);
	vec3 normal = normalize(texture(sampler2D(Fnormals, Fsampler), Fuv).rgb * 2.0f - 1.0f);

	vec3 view_dir = normalize(-Fpos);

	vec3 total = calc_dir_light(global.light, normal, view_dir, albedo.rgb);
	for (int i = 0; i < chunks.data[Fchunk_id].spotc; i++)
		total += calc_spot_light(spot_lights.data[i], normal, Fpos, view_dir, albedo.rgb);
	for (int i = 0; i < chunks.data[Fchunk_id].pointc; i++)
		total += calc_point_light(point_lights.data[i], normal, Fpos, view_dir, albedo.rgb);

	Foutput = vec4(total, albedo.a);
}
