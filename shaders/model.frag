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
layout(binding = 3) uniform LightBuffer {
	float ambient;
	float power;
	vec3  dir;
	vec3  colour;
} global_light;

layout(push_constant) uniform PushConstants
{
	int   materiali;
	float timer;
} constants;

// finalColor = ambient
//           + lambertianTerm * surfaceColor * lightColor
//           + specularIntensity * specularColor * lightColor;
void main()
{
	vec3 ambient = global_light.colour * global_light.ambient;

	rgba = vec4(materials.materials[constants.materiali].albedo.xyz * ambient, 1.0);
	// rgba = vec4(Fnnn, 1.0);
}
