#version 460
#extension GL_GOOGLE_include_directive: enable

#include "common.glsl"

layout(location = 0) in vec3 Fpos;
layout(location = 1) in vec3 Fnormal;
layout(location = 2) in vec2 Fuv;

layout(location = 0) out vec4 Foutput;

layout(binding = 20) uniform sampler   Fsampler;
layout(binding = 21) uniform texture2D Fdiffuse;
layout(binding = 22) uniform texture2D Fnormals;

struct Light {
	vec4 ambient; /* [colour|power] */
	vec4 pos;     /* [pos|power]    */
	vec3 colour;
	uint id;
};
layout(binding = 1) uniform LightBufferUBO {
	Light global;
	Light local[MAX_LIGHTS];
} lights;

void main()
{
	vec4 diffuse_colour = texture(sampler2D(Fdiffuse, Fsampler), Fuv);
	vec4 normal_colour  = texture(sampler2D(Fnormals, Fsampler), Fuv);
	vec3 normal = normalize(normal_colour.rgb * 2.0f - 1.0f);

	vec3 ambient = lights.global.ambient.xyz * lights.global.ambient.w;

	vec3 light_dir = normalize(-lights.global.pos.xyz - Fpos);
	// vec3 diffuse   = max(-dot((Fnormal + normal)/2.0f, light_dir), 0.0f) * lights.global.colour * lights.global.pos.w;
	vec3 diffuse   = max(-dot(normal, light_dir), 0.0f) * lights.global.colour * lights.global.pos.w;
	diffuse *= diffuse_colour.xyz;

	// TODO: this in a buffer
	float spec_str   = 0.5f;
	float shininess  = 32.0f;
	vec3 view_dir    = normalize(-Fpos);
	vec3 halfway_dir = -normalize(light_dir + view_dir);
	// vec3 specular    = lights.global.colour * spec_str * pow(max(dot(Fnormal, halfway_dir), 0.0f), shininess);
	vec3 specular    = lights.global.colour * spec_str * pow(max(dot(normal, halfway_dir), 0.0f), shininess);
	specular *= diffuse_colour.xyz;

	Foutput = vec4(ambient + diffuse + specular, diffuse_colour.w);
	// Foutput = vec4(Fnormal, 1.0);
	// Foutput = vec4(normal, 1.0);
}
