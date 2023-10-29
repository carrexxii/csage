#version 460

layout(location = 0) in vec2 Fuv;
layout(location = 1) in vec3 Fnnn;

layout(location = 0) out vec4 rgba;

struct Material {
	vec4  albedo;
	float metallic;
	float roughness;
};
layout(binding = 2) uniform MaterialsBuffer {
	Material materials[8];
} materials;

layout(push_constant) uniform PushConstants
{
	int   materiali;
	float timer;
} constants;

void main()
{
	// rgba = vec4(materials.materials[constants.materiali].albedo);
	rgba = vec4(Fnnn, 1.0);
}
