#version 460
#extension GL_GOOGLE_include_directive : enable

#include "common.glsl"

const int SELECTION_COUNT = 16;

layout(location = 0) in vec3 Fpos;
layout(location = 1) in vec3 Fnormal;

layout(location = 0) out vec4 FragColour;

layout(binding = 1) uniform SelectionDataUBO {
	vec4 pos[SELECTION_COUNT];
} selections;

void main()
{
	FragColour = vec4(Fnormal, 1.0);
	vec4 sel;
	// TODO: selection count -> push constant
	for (int i = 0; i < SELECTION_COUNT; i++) {
		sel = selections.pos[i];
		if (is_in_rect(Fpos.xy, sel.xy, sel.xy + sel.zw))
			FragColour = vec4(0.1, 0.1, 0.1, 1.0);
	}
}
