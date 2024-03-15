#version 460

#include "common.glsl"

layout(location = 0) out vec3 Fpos;
layout(location = 1) out vec2 Fuv;

layout(binding = 0) uniform CameraUBO {
	mat4 proj;
	mat4 view;
} cam;

void main()
{
	vec3 pos = rect_verts[gl_VertexIndex % 6];

	Fpos = pos;
	Fuv  = pos.xz;

	gl_Position = cam.proj * cam.view * vec4(pos - vec3(0, 0, 1), 1.0f);
}
