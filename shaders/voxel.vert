#version 460

layout(location = 0) in vec3 Vxyz;

layout(location = 0) out vec3 Gxyz;

layout(binding = 0) uniform UniformBufferObject {
	mat4 vp;
} cam;

void main()
{
	gl_Position = cam.vp * vec4(Vxyz, 1.0);
	Gxyz = Vxyz;

	/* glm (OpenGL) -> Vulkan depth */
	gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}
