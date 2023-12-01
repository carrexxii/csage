#version 460

layout(location = 0) in ivec3 Vpos;
layout(location = 1) in ivec3 Vnormal;

layout(location = 0) out vec3 Fpos;
layout(location = 1) out vec3 Fnormal;

// struct ObjectData {
// 	mat4 mat;
// };
// layout(std140, binding = 0) readonly buffer ObjectBuffer {
// 	ObjectData objs[];
// } mdls;

layout(binding = 0) uniform MapDataUBO {
	mat4  cam_vp;
	ivec4 dim;
	ivec4 block_dim;
} map;

layout(push_constant) uniform PushConstants {
	int i;
} constants;

void main()
{
	Fpos    = vec3(Vpos);
	Fnormal = vec3(Vnormal);
	
	vec3 block_pos = vec3(mod(constants.i, map.dim.x) * map.block_dim.x,
	                      mod((constants.i / map.dim.y), map.dim.y) * map.block_dim.y,
	                      constants.i / (map.dim.x*map.dim.y) * map.block_dim.z);

	gl_Position = map.cam_vp * vec4(Vpos + vec3(-0.5, -0.5, 0.0) + block_pos*1.1, 1.0);
}
