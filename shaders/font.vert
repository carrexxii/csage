#version 460

layout(location = 0) in vec2 Vxy;
layout(location = 1) in vec2 Vuv;

layout(location = 0) out vec2 Fuv;

layout(push_constant) uniform PushConstants {
	float z_lvl;
	vec2  pos;
} constants;

void main()
{
	Fuv = Vuv;

	gl_Position = vec4(Vxy + constants.pos, 0.0f, 1.0f);
	gl_Position.y *= -1.0f;
}
