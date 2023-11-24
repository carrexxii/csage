#version 460

layout(location = 0) in vec2 Fuv;
layout(location = 1) in vec3 Fnormal;
layout(location = 2) in vec3 Fpos;

layout(location = 0) out vec4 FragColor;

struct Material {
	vec4  albedo;
	float metallic;
	float roughness;
};
layout(binding = 2) uniform MaterialsBuffer {
	Material materials[8];
} materials;
layout(binding = 3) uniform LightBuffer {
	vec4 ambient; /* [colour|power] */
	vec4 pos;     /* [pos|power]    */
	vec3 colour;
} global_light;

layout(push_constant) uniform PushConstants {
	int   materiali;
	float timer;
} constants;

// finalColor = ambient
//           + lambertianTerm * surfaceColor * lightColor
//           + specularIntensity * specularColor * lightColor;
void main()
{
	vec3 ambient   = global_light.ambient.xyz * global_light.ambient.w;
	// vec3 light_dir = normalize(normalize(global_light.pos.xyz) - Fnormal);
	vec3 light_dir = normalize(-global_light.pos.xyz - Fpos);
	vec3 diffuse   = max(-dot(Fnormal, light_dir), 0.0) * global_light.colour * global_light.pos.w;
	// diffuse = global_light.colour;

	vec4 obj_colour = materials.materials[constants.materiali].albedo;
	obj_colour = vec4(0.3, 0.3, 0.3, 1.0);
	FragColor = vec4((ambient + diffuse)*obj_colour.xzy, obj_colour.w);
	// FragColor = vec4(max(-dot(Fnormal, light_dir), 0.0) * global_light.colour, 1.0);
	// FragColor = vec4(diffuse, 1.0);
	// FragColor = vec4((Fnormal + vec3(1.0, 1.0, 1.0))/2.0, 1.0);

	//     // ambient
    // float ambientStrength = 0.1;
    // vec3 ambient = ambientStrength * lightColor;
  	
    // // diffuse 
    // vec3 norm = normalize(Normal);
    // vec3 lightDir = normalize(lightPos - FragPos);
    // float diff = max(dot(norm, lightDir), 0.0);
    // vec3 diffuse = diff * lightColor;
            
    // vec3 result = (ambient + diffuse) * objectColor;
    // FragColor = vec4(result, 1.0);
}
