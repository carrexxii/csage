#version 460

layout(location = 0) in ivec3 Vxyz;
layout(location = 1) in ivec3 Vnnn;

layout(location = 0) out vec3 Fxyz;
layout(location = 1) out vec3 Fnnn;

// struct ObjectData {
// 	mat4 mat;
// };
// layout(std140, binding = 0) readonly buffer ObjectBuffer {
// 	ObjectData objs[];
// } mdls;

layout(binding = 0) uniform UniformBuffer {
	mat4  cam_vp;
	ivec4 map_dim;
	ivec4 block_dim;
} ubo;

layout(push_constant) uniform PushConstants {
	int i;
} constants;

void main()
{
	Fxyz = vec3(Vxyz);
	Fnnn = vec3(Vnnn);
	
	vec3 block_pos = vec3(mod(constants.i, ubo.map_dim.x) * ubo.block_dim.x,
	                      mod((constants.i / ubo.map_dim.y), ubo.map_dim.y) * ubo.block_dim.y,
	                      constants.i / (ubo.map_dim.x*ubo.map_dim.y) * ubo.block_dim.z);

	gl_Position = ubo.cam_vp * vec4(Vxyz + block_pos*1.1, 1.0);
}
