#version 460

#include "common.glsl"

const ivec2 tex_tile_dim = ivec2(4096 / 128);
const ivec2 block_dim    = ivec2(16);
const int   block_size   = block_dim.x*block_dim.y;

layout(location = 0) in uvec3 Vpos;
layout(location = 1) in uint  Vtile;
layout(location = 2) in uvec2 Vuv;

layout(location = 0) out vec3 Fpos;
layout(location = 1) out vec2 Fuv;
layout(location = 2) out uint Fchunk_id;

layout(binding = 0) uniform CameraUBO {
	mat4 proj;
	mat4 view;
} cam;

layout(binding = 2) uniform MapDataUBO {
	ivec2 chunk_dim;
	ivec2 tex_dim;
} map;

layout(binding = 10) readonly buffer ChunkDataSBO {
	MapChunkData data[];
} chunks;

layout(push_constant) uniform PushConstants {
	ivec2 block_pos;
} push;

void main()
{
	uint chunk_id = push.block_pos.y*map.chunk_dim.x + push.block_pos.x;
	uint tile_id  = chunks.data[chunk_id].tiles[Vtile] - 1;
	vec2 uv = vec2(tile_id % tex_tile_dim.x * 4,
	               tile_id / tex_tile_dim.y * 4);
	vec3 global_pos = vec3(push.block_pos * block_dim, 0.0f);

	Fpos = vec3(Vpos + global_pos);
	Fuv  = (Vuv + uv) / 4.0f / tex_tile_dim;
	Fchunk_id = chunk_id;

	gl_Position = cam.proj * cam.view * vec4(Vpos + global_pos, 1.0f);
}
