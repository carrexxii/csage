#version 460

layout(location = 0) in vec3 Vxyz;
layout(location = 1) in vec3 Vrgb;
layout(location = 2) in vec3 Vnormal;

layout(location = 0) out vec3 Fxyz;
layout(location = 1) out vec3 Frgb;
layout(location = 2) out vec3 Fnormal;

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
	mat4 mdl = mdls.objs[gl_BaseInstance].mat;
	gl_Position = cam.vp * mdl * vec4(Vxyz, 1.0);
	Fxyz    = Vxyz;
	Frgb    = Vrgb;
	Fnormal = Vnormal;

	gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}
