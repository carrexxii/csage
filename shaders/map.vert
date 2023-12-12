#version 460

layout(location = 0) in ivec3 Vpos;
layout(location = 1) in ivec3 Vnormal;

layout(location = 0) out vec3 Fpos;
layout(location = 1) out vec3 Fnormal;

layout(binding = 0) uniform MapDataUBO {
	mat4  proj;
	mat4  view;
	ivec4 dim;
	ivec4 block_dim;
} map;

layout(push_constant) uniform PushConstants {
	int blocki;
	int selc;
} consts;

void main()
{
	vec3 block_pos = vec3(mod(consts.blocki, map.dim.x),
	                      mod((consts.blocki / map.dim.y), map.dim.y),
	                      consts.blocki / (map.dim.x*map.dim.y));
	vec3 global_pos = block_pos * map.block_dim.xyz;

	Fpos    = vec3(Vpos + global_pos);
	Fnormal = vec3(Vnormal);

	gl_Position = map.proj * map.view * vec4(Vpos, 1.0);
}
