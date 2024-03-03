#version 460

layout(location = 0) in vec3 Fpos;
layout(location = 1) in vec3 Fnormal;
layout(location = 2) in vec2 Fuv;

layout(location = 0) out vec4 Foutput;

layout (binding = 20) uniform sampler   Fsampler;
layout (binding = 21) uniform texture2D Ftexture;

void main()
{
	vec4 tex_colour = texture(sampler2D(Ftexture, Fsampler), Fuv);
	Foutput = tex_colour;
}
