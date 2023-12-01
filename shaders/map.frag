#version 460

const int SELECTION_COUNT = 64;

layout(location = 0) in vec3 Fpos;
layout(location = 1) in vec3 Fnormal;

layout(location = 0) out vec4 FragColour;

layout(binding = 1) uniform SelectionDataUBO {
	ivec4 pos[SELECTION_COUNT];
} selections;

void main()
{
	FragColour = vec4(Fnormal, 1.0);
	// TODO: replace with a separate draw call/pipeline
	ivec4 sel;
	for (int i = 0; i < SELECTION_COUNT; i++) {
		sel = selections.pos[i];
		if ((Fpos.x >= sel.x && Fpos.x <= sel.x + sel.z) &&
			(Fpos.y >= sel.y && Fpos.y <= sel.y + sel.w))
			FragColour = vec4(0.1, 0.1, 0.1, 1.0);
	}
}
