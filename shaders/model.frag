#version 460

layout(location = 0) in vec2 Fuv;
layout(location = 1) in vec3 Fnormal;
layout(location = 2) in vec3 Fpos;

layout(location = 0) out vec4 FragColor;

struct Material {
	vec4  albedo;
	float metallic;
	float roughness;
	vec4 pad0;
	vec4 pad1;
};
layout(set = 0, binding = 3) uniform MaterialsBufferUBO {
	Material materials[8];
} materials;
layout(set = 0, binding = 4) uniform LightBufferUBO {
	vec4 ambient; /* [colour|power] */
	vec4 pos;     /* [pos|power]    */
	vec3 colour;
} global_light;

layout(push_constant) uniform PushConstants {
	int   materiali;
	float timer;
} constants;

layout (set = 0, binding = 0) uniform sampler   Fsampler;
layout (set = 1, binding = 0) uniform texture2D Ftexture;

void main()
{
	vec4 obj_colour = materials.materials[constants.materiali].albedo;

	vec3 ambient   = obj_colour.xyz*(global_light.ambient.xyz * global_light.ambient.w);

	vec3 light_dir = normalize(-global_light.pos.xyz - Fpos);
	vec3 diffuse   = max(-dot(Fnormal, light_dir), 0.0) * global_light.colour * global_light.pos.w;
	diffuse *= obj_colour.xyz;

	// TODO: this in a buffer
	float spec_str   = 0.5;
	float shininess  = 32;
	vec3 view_dir    = normalize(-Fpos);
	vec3 halfway_dir = -normalize(light_dir + view_dir);
	vec3 specular    = global_light.colour * spec_str * pow(max(dot(Fnormal, halfway_dir), 0.0), shininess);
	specular *= obj_colour.xyz;

	// obj_colour = texture(sampler2D(Ftexture, Fsampler), Fuv);
	FragColor = vec4(ambient + diffuse + specular, obj_colour.w);
}
