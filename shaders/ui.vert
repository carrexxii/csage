#version 460

layout(location = 0) in vec3 Vpos;
layout(location = 1) in vec4 Vcolour;

layout(location = 0) out vec4 Fcolour;

void main()
{
	Fcolour = Vcolour;

	gl_Position = vec4(Vpos.xy, 1.0/Vpos.z, 1.0);
	gl_Position.y *= -1.0;
}
