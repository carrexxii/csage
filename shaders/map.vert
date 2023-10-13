#version 460

layout(location = 0) in ivec3 Vxyz;
layout(location = 1) in ivec3 Vnnn;

layout(location = 0) out vec3 Fnnn;

// struct ObjectData {
// 	mat4 mat;
// };
// layout(std140, binding = 0) readonly buffer ObjectBuffer {
// 	ObjectData objs[];
// } mdls;

layout(binding = 0) uniform CameraBuffer {
	mat4 vp;
} cam;

void main()
{
	Fnnn = vec3(Vnnn);
	
	gl_Position = cam.vp * vec4(Vxyz, 1.0);
	gl_Position.y = -gl_Position.y;
}
