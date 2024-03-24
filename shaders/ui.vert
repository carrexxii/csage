#version 460

layout(location = 0) in vec2  Vpos;
layout(location = 1) in uvec4 Vcolour;

layout(location = 0) out vec4 Fcolour;

void main()
{
	Fcolour = vec4(Vcolour.abgr) / 255.0f;

	float z = 0.01f - 0.0001f * (gl_VertexIndex / 6);
	gl_Position = vec4(Vpos.xy, z, 1.0f);
}
