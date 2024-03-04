#version 460

const ivec2 tex_tile_dim = ivec2(32, 32);
const ivec3 block_dim    = ivec3(16, 16, 1);
const int   block_size   = block_dim.x*block_dim.y*block_dim.z;

layout(location = 0) in uvec3 Vpos;
layout(location = 1) in uint  Vnormal;
layout(location = 2) in uint  Vtile;
layout(location = 3) in uvec2 Vuv;

layout(location = 0) out vec3 Fpos;
layout(location = 1) out vec3 Fnormal;
layout(location = 2) out vec2 Fuv;

layout(binding = 0) uniform CameraBufferUBO {
	mat4 proj;
	mat4 view;
} cam;
layout(binding = 2) uniform MapDataUBO {
	uvec4 block_data[block_size / 4];
} map;

layout(push_constant) uniform PushConstants {
	ivec3 block_pos;
} consts;

const vec3 normals[3] = {
	vec3(1.0f, 0.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(0.0f, 0.0f, 1.0f),
};

void main()
{
	uint tile_id    = map.block_data[Vtile / 4][int(Vtile % 4)];
	uint tex_x      = (tile_id - 1) % tex_tile_dim.x * 4;
	uint tex_y      = (tile_id - 1) / tex_tile_dim.x * 4;
	vec3 global_pos = consts.block_pos * block_dim;

	Fpos = vec3(Vpos + global_pos);
	Fuv  = (Vuv + vec2(tex_x, tex_y)) / 4.0f / tex_tile_dim;
	Fnormal = normals[Vnormal];

	gl_Position = cam.proj * cam.view * vec4(Vpos + global_pos, 1.0f);
}
