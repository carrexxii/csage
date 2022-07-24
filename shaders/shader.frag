#version 450

layout(location = 0) in vec3 Fxyz;
layout(location = 1) in vec3 Frgb;
layout(location = 2) in vec3 Fnormal;

layout(location = 0) out vec4 screen_colour;

layout(binding = 1) uniform UniformBufferObject {
    vec3  sundir;
    float ambient;
    float sunpower;
} lighting;

void main()
{
    vec3 diffuse  = vec3(max(dot(Fnormal, lighting.sundir), 0.0));
    screen_colour = vec4(Frgb * (lighting.ambient + diffuse), 1.0);
    // screen_colour = vec4(abs(Fnormal) * (lighting.ambient + diffuse), 1.0);
}
