#version 460

layout(location = 0) in vec2 Fuv;

layout(location = 0) out vec4 screen_colour;

layout (binding = 20) uniform sampler   Fsampler;
layout (binding = 21) uniform texture2D Ftexture;

void main()
{
	vec4 tex = texture(sampler2D(Ftexture, Fsampler), Fuv);
	screen_colour = tex * vec4(0.0, 0.0, 0.0, 1.0);
}
