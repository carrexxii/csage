#version 460

layout(location = 0) in vec2 Vxy;
layout(location = 1) in vec3 Vrgb;

layout(location = 0) out vec3 Frgb;

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
	Frgb = Vrgb;
	
	mat4 mdl = mdls.objs[gl_BaseInstance].mat;
	gl_Position = cam.vp * mdl * vec4(Vxy.x, Vxy.y, 0.0, 1.0);
	gl_Position.y = -gl_Position.y;
}
