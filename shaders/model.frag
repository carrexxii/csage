#version 460
#extension GL_EXT_nonuniform_qualifier: enable

#include "common.glsl"

layout(location = 0) in vec3 Fpos;
layout(location = 1) in vec3 Fnormal;
layout(location = 2) in vec2 Fuv;

layout(location = 0) out vec4 Foutput;

layout(binding = 0) uniform CameraUBO {
	mat4 proj;
	mat4 view;
} cam;

layout(binding = 1) uniform GlobalLightBufferUBO {
	DirectionalLight light;
} global;

// layout(binding = 12) readonly buffer SpotLightDataSBO {
// 	int count;
// 	SpotLight data[];
// } spot_lights;

// layout(binding = 13) readonly buffer PointLightDataSBO {
// 	int count;
// 	PointLight data[];
// } point_lights;

layout(binding = 11) readonly buffer MaterialsBufferSBO {
	Material data[];
} mtls;

layout(push_constant) uniform PushConstants {
	int   mtli;
	float timer;
} constants;

layout (binding = 20) uniform sampler   Fsampler;
layout (binding = 21) uniform texture2D Ftexture[];

void main()
{
	vec4 obj_colour = mtls.data[constants.mtli].albedo;

	// vec3 ambient   = obj_colour.xyz*(global_light.ambient.xyz * global_light.ambient.w);

	// vec3 light_dir = normalize(-global_light.pos.xyz - Fpos);
	// vec3 diffuse   = max(-dot(Fnormal, light_dir), 0.0) * global_light.colour * global_light.pos.w;
	// diffuse *= obj_colour.xyz;

	// // TODO: this in a buffer
	// float spec_str   = 0.5;
	// float shininess  = 32;
	// vec3 view_dir    = normalize(-Fpos);
	// vec3 halfway_dir = -normalize(light_dir + view_dir);
	// vec3 specular    = global_light.colour * spec_str * pow(max(dot(Fnormal, halfway_dir), 0.0), shininess);
	// specular *= obj_colour.xyz;

	// obj_colour = texture(sampler2D(Ftexture, Fsampler), Fuv);
	// Foutput = obj_colour;
	// Foutput = vec4(ambient + diffuse + specular, obj_colour.w);
	Foutput = vec4(1.0, 0.0, 0.0, 1.0);
}

