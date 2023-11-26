#version 460

layout(location = 0) in vec4 Fcolour;

layout(location = 0) out vec4 screen_colour;

layout (binding = 0) uniform sampler Fsampler;

void main()
{
	screen_colour = Fcolour;
}
