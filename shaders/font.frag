#version 460

layout(location = 0) in vec2 Fuv;

layout(location = 0) out vec4 screen_colour;

layout (set = 0, binding = 1) uniform sampler   Fsampler;
layout (set = 0, binding = 2) uniform texture2D Ftexture;

void main()
{
	screen_colour = texture(sampler2D(Ftexture, Fsampler), Fuv);
}
