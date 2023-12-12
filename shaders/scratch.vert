#version 460

layout(location = 0) in vec3 Vpos;
layout(location = 1) in vec4 Vcolour;

layout(location = 0) out vec4 Fcolour;

layout(binding = 0) uniform CameraBufferUBO {
	mat4 proj;
	mat4 view;
} cam;

void main()
{
	Fcolour = Vcolour;

	gl_Position  = cam.proj * cam.view * vec4(Vpos, 1.0);
	gl_PointSize = 10.0f;
}
