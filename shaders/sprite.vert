#version 460

layout(location = 0) out vec3 Fpos;
layout(location = 1) out vec2 Fuv;

layout(binding = 0) uniform CameraUBO {
	mat4 proj;
	mat4 view;
} cam;

layout(push_constant) uniform PushConstants {
	int id;
} push;

void main()
{
	vec3 pos = vec3(0, 0, 0);

	Fpos = pos;
	vec2 Fuv = vec2(0, 0);

	gl_Position = cam.proj * cam.view * vec4(pos, 1.0f);
}
