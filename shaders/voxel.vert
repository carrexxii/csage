#version 460

layout(location = 0) in uvec4 Vxyza;

layout(location = 0) out vec3 Gxyz;

struct MapData {
	uvec3 dim;
	uvec3 stride;
	// vec3 materials[255];
};
layout(std140, binding = 0) readonly buffer ObjectBuffer {
	MapData data;
} map;

layout(binding = 1) uniform UniformBufferObject {
	mat4 vp;
} cam;

vec4 xyza;

void main()
{
	xyza = vec4(Vxyza);
	xyza.x += 1.1*float(gl_InstanceIndex % map.data.dim.x * map.data.stride.x);
	xyza.y += 1.1*float(gl_InstanceIndex / map.data.dim.x * map.data.stride.y);
	gl_Position = cam.vp * xyza;
	Gxyz = vec3(Vxyza);

	/* glm (OpenGL) -> Vulkan depth */
	gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}
