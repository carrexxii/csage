#version 460

layout(location = 0) in vec3 Vxyz;
layout(location = 1) in vec3 Vnnn;
layout(location = 2) in vec2 Vuv;

layout(location = 0) out vec2 Fuv;

struct ObjectData {
	mat4 mat;
};
layout(std140, binding = 0) readonly buffer ObjectBuffer {
	ObjectData objs[];
} mdls;

layout(binding = 1) uniform UniformBufferObject {
	mat4 vp;
} cam;

void main()
{
	// Fuv = Vuv;
	Fuv = Vxyz.yz;
	
	mat4 mdl = mdls.objs[gl_BaseInstance].mat;
	gl_Position = cam.vp * mdl * vec4(Vxyz, 1.0);
	// gl_Position.y = -gl_Position.y;
}
