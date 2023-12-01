#version 460

layout(location = 0) in vec2 Fuv;

layout(location = 0) out vec4 screen_colour;

layout (binding = 20) uniform sampler   Fsampler;
layout (binding = 21) uniform texture2D Ftexture;

void main()
{
    vec3 Frgb = vec3(0.3, 0.7, 1.0);
    screen_colour = texture(sampler2D(Ftexture, Fsampler), Fuv) * vec4(Frgb, 1.0);
}
