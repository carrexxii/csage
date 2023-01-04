#version 460

layout(location = 0) in ivec4 Vxyza;

layout(location = 0) out vec3 Gxyz;

struct MapData {
	ivec3 dim;
	ivec3 stride;
	ivec3 selection; // z-component should indicate if it is active or not
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
	xyza.x += 1.1*float(gl_InstanceIndex  %  map.data.dim.x * map.data.stride.x);
	xyza.y += 1.1*float((gl_InstanceIndex % (map.data.dim.y * map.data.dim.x) / map.data.dim.x * map.data.stride.y));
	xyza.z += 1.1*float(gl_InstanceIndex  / (map.data.dim.x * map.data.dim.y) * map.data.stride.z);
	gl_Position = cam.vp * xyza;
	Gxyz = vec3(Vxyza);

	gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}
