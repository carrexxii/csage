#version 460

layout(location = 0) in uvec4 Vpos;

layout(location = 0) out vec3 Fpos;
layout(location = 1) out vec3 Fnormal;

layout(binding = 0) uniform CameraBufferUBO {
	mat4 proj;
	mat4 view;
} cam;
layout(binding = 1) uniform MapDataUBO {
	ivec3 block_size;
} map;

layout(push_constant) uniform PushConstants {
	ivec3 block_pos;
} consts;

const vec3 normals[] = {
	vec3(1.0f, 0.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(0.0f, 0.0f, 1.0f),
};

void main()
{
	vec3 global_pos = consts.block_pos * map.block_size;

	Fpos    = vec3(Vpos.xyz + global_pos);
	Fnormal = vec3(normals[Vpos.w]);

	gl_Position = cam.proj * cam.view * vec4(Vpos.xyz + global_pos, 1.0f);
	
}
