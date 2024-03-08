#version 460

const ivec2 tex_tile_dim = ivec2(4096 / 128);
const ivec2 block_dim    = ivec2(16);
const int   block_size   = block_dim.x*block_dim.y;

layout(location = 0) in uvec3 Vpos;
layout(location = 1) in uint  Vtile;
layout(location = 2) in uvec2 Vuv;

layout(location = 0) out vec3 Fpos;
layout(location = 1) out vec2 Fuv;

layout(binding = 0) uniform CameraBufferUBO {
	mat4 proj;
	mat4 view;
} cam;

layout(binding = 10) readonly buffer MapDataSBO {
	int block_width;
	int block_height;
	uint data[];
} map;

layout(push_constant) uniform PushConstants {
	ivec2 block_pos;
} push;

void main()
{
	uint tile_id = map.data[push.block_pos.y * block_size * map.block_width +
	                        push.block_pos.x * block_size +
	                        Vtile] - 1;
	vec2 uv = vec2(tile_id % tex_tile_dim.x * 4,
	               tile_id / tex_tile_dim.y * 4);
	vec3 global_pos = vec3(push.block_pos * block_dim, 0.0f);

	Fpos = vec3(Vpos + global_pos);
	Fuv  = (Vuv + uv) / 4.0f / tex_tile_dim;

	gl_Position = cam.proj * cam.view * vec4(Vpos + global_pos, 1.0f);
}
