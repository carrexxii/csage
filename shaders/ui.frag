#version 460
#extension GL_EXT_nonuniform_qualifier: enable

#include "common.glsl"

layout(location = 0) in vec2 Fpos;
layout(location = 1) in vec4 Fcolour;
layout(location = 2) in flat int Ftex_id;
layout(location = 3) in vec2 Fuv;
layout(location = 4) in vec4 Fhl;

layout(location = 0) out vec4 screen_colour;

layout(binding = 20) uniform sampler   Fsampler;
layout(binding = 21) uniform texture2D Ftextures[];

void main()
{
	vec3 hl = vec3(0.5 * int(is_in_rect(Fpos, Fhl.xy, Fhl.xy + Fhl.zw)));

	if (Ftex_id != -1) {
		vec4 texture = texture(sampler2D(Ftextures[Ftex_id], Fsampler), Fuv);
		screen_colour = Fcolour + texture;
	} else {
		screen_colour = Fcolour;
	}
	screen_colour += vec4(hl, 0.0f);
}
