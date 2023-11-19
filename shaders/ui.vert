#version 460

layout(location = 0) in vec3 Vpos;
layout(location = 1) in vec4 Vcolour;

layout(location = 0) out vec4 Fcolour;

void main()
{
	Fcolour = Vcolour;

	gl_Position = vec4(Vpos - vec3(1.0, 1.0, 0.0), 1.0);
}
