#version 460

layout(location = 0) in uvec4 Vxyza;

layout(location = 0) out vec3 Gxyz;

struct MapData {
	uvec3 blockstride;
	// vec3 materials[255];
};
layout(std140, binding = 0) readonly buffer ObjectBuffer {
	MapData data;
} map;

layout(binding = 1) uniform UniformBufferObject {
	mat4 vp;
} cam;

void main()
{
	gl_Position += gl_InstanceIndex * vec4(0.0, 0.0, 1.0, 0.0);//vec4(map.data.blockstride, 0.0);
	gl_Position = cam.vp * Vxyza;
	Gxyz = vec3(Vxyza);

	/* glm (OpenGL) -> Vulkan depth */
	gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}
