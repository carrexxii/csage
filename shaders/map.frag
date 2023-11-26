#version 460

#define SELECTION_COUNT 64

layout(location = 0) in vec3 Fxyz;
layout(location = 1) in vec3 Fnnn;

layout(location = 0) out vec4 rgba;

layout (set = 0, binding = 0) uniform sampler Fsampler;

layout(set = 0, binding = 2) uniform UniformBuffer {
	ivec4 selections[SELECTION_COUNT];
} ubo;

void main()
{
	rgba = vec4(Fnnn, 1.0);
	// TODO: replace with a separate draw call/pipeline
	ivec4 sel;
	for (int i = 0; i < SELECTION_COUNT; i++) {
		sel = ubo.selections[i];
		if ((Fxyz.x >= sel.x && Fxyz.x <= sel.x + sel.z) &&
			(Fxyz.y >= sel.y && Fxyz.y <= sel.y + sel.w))
			rgba = vec4(0.1, 0.1, 0.1, 1.0);
	}
}
