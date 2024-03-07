#version 460

#include "common.glsl"

layout(location = 0) in vec3 Fpos;
layout(location = 1) in vec3 Fnormal;
layout(location = 2) in vec2 Fuv;

layout(location = 0) out vec4 Foutput;

layout(binding = 20) uniform sampler   Fsampler;
layout(binding = 21) uniform texture2D Falbedo;
layout(binding = 22) uniform texture2D Fnormals;

layout(binding = 1) uniform GlobalLightBufferUBO {
	vec4 dir;
	vec4 ambient;
} global;
layout(binding = 2) uniform LightBufferUBO {
	Light lights[MAX_LIGHTS];
} local;

void main()
{
	vec4 albedo = texture(sampler2D(Falbedo, Fsampler), Fuv);
	vec3 normal = normalize(texture(sampler2D(Fnormals, Fsampler), Fuv).rgb * 2.0f - 1.0f);

	vec3 ambient = global.ambient.xyz * global.ambient.w;

	// vec3 light_dir = normalize(-global.dir.xyz - Fpos);
	vec3 light_dir = normalize(VIEW_DIR - global.dir.xyz);
	// vec3 light_dir = normalize(global.dir.xyz);
	// light_dir.x = -light_dir.x;
	light_dir.y = -light_dir.y;
	// light_dir.y = -light_dir.y;
	// vec3 diffuse   = max(-dot((Fnormal + normal)/2.0f, light_dir), 0.0f) * global.colour * global.pos.w;
	// vec3 diffuse   = max(-dot(normal, light_dir), 0.0f) * global.ambient.xyz * global.dir.w;
	vec3 diffuse   = max(-dot(normal, light_dir), 0.0f) * global.ambient.xyz * global.dir.w;
	diffuse *= albedo.xyz;
	// diffuse = vec3(0);

	Light light;
	for (int i = 0; i < MAX_LIGHTS; i++) {
		light = local.lights[i];
		float dist = length(Fpos - light.pos.xyz);
		if (dist < light.pos.w) {
			light_dir = normalize(light.pos.xyz - Fpos);
			diffuse += max(-dot(normal, light_dir), 0.0f) * (light.pos.w / dist) * light.colour.w;
		}
	}

	// // TODO: this in a buffer
	// float spec_str   = 0.5f;
	// float shininess  = 32.0f;
	// vec3 view_dir    = normalize(-Fpos);
	// vec3 halfway_dir = -normalize(light_dir + view_dir);
	// // vec3 specular    = global.colour * spec_str * pow(max(dot(Fnormal, halfway_dir), 0.0f), shininess);
	// vec3 specular    = global.colour * spec_str * pow(max(dot(normal, halfway_dir), 0.0f), shininess);
	// specular *= albedo.xyz;
	vec3 specular = vec3(0);

	Foutput = vec4(ambient + diffuse + specular, albedo.w);
	// Foutput = vec4(Fnormal, 1.0);
	// Foutput = vec4(normal, 1.0);
}
