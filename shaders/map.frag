#version 460
#extension GL_GOOGLE_include_directive: enable

#include "common.glsl"

const int  SELECTION_MAX    = 16;
const vec4 SELECTION_COLOUR = vec4(0.10, 0.10, 0.10, 1.0);

layout(location = 0) in vec3 Fpos;
layout(location = 1) in vec3 Fnormal;

layout(location = 0) out vec4 FragColour;

layout(binding = 0) uniform MapDataUBO {
	mat4  cam_vp;
	ivec4 dim;
	ivec4 block_dim;
} map;

layout(binding = 1) uniform SelectionDataUBO {
	vec4 pos[SELECTION_MAX];
} selections;

layout(push_constant) uniform PushConstants {
	int blocki;
	int selc;
} consts;

void main()
{
	FragColour = vec4(Fnormal, 1.0);

	vec4 sel;
	for (int i = 0; i < consts.selc; i++) {
		sel = selections.pos[i];
		if (is_in_rect(Fpos.xy, sel.xy, sel.xy + sel.zw))
			FragColour = SELECTION_COLOUR;
	}
}
