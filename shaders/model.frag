#version 460

layout(location = 0) in vec3 Fxyz;
layout(location = 1) in vec3 Frgb;
layout(location = 2) in vec3 Fnormal;

layout(location = 0) out vec4 screen_colour;

layout(binding = 2) uniform UniformBufferObject {
    vec3  sundir;
    float ambient;
    float sunpower;
    vec4  lights[];
} lighting;

void main()
{
    vec3 diffuse = vec3(max(dot(Fnormal, lighting.sundir), 0.0) * lighting.sunpower);
    diffuse += vec3(max(dot(Fnormal, vec3(lighting.lights[0])), 0.0) * lighting.lights[0].w);
    screen_colour = vec4(Frgb * (lighting.ambient + diffuse), 1.0);
    // screen_colour = vec4(abs(Fnormal) * (lighting.ambient + diffuse), 1.0);
}

