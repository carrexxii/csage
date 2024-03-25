#version 460
#extension GL_EXT_nonuniform_qualifier: enable

layout(location = 0) in vec4 Fcolour;
layout(location = 1) in flat int Ftex_id;
layout(location = 2) in vec2 Fuv;

layout(location = 0) out vec4 screen_colour;

layout(binding = 20) uniform sampler   Fsampler;
layout(binding = 21) uniform texture2D Ftextures[];

void main()
{
	if (Ftex_id != -1) {
		vec4 texture = texture(sampler2D(Ftextures[Ftex_id], Fsampler), Fuv);
		screen_colour = Fcolour + texture;
	} else {
		screen_colour = Fcolour;
	}
}
