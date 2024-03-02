#version 460

layout(location = 0) in ivec3 Vpos;
layout(location = 1) in ivec3 Vnormal;

layout(location = 0) out vec3 Fpos;
layout(location = 1) out vec3 Fnormal;

layout(binding = 0) uniform CameraBufferUBO {
	mat4 proj;
	mat4 view;
} cam;
layout(binding = 1) uniform MapDataUBO {
	mat4  proj;
	mat4  view;
	ivec3 block_size;
} map;

layout(push_constant) uniform PushConstants {
	ivec3 block_pos;
} consts;

void main()
{
	vec3 global_pos = consts.block_pos * map.block_size;

	Fpos    = vec3(Vpos + global_pos);
	Fnormal = vec3(Vnormal);

	gl_Position = cam.proj * cam.view * vec4(Vpos + global_pos, 1.0f);
}
