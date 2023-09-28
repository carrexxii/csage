#version 460

layout(location = 0) in vec2 Fxyz;
layout(location = 1) in vec3 Frgb;

layout(location = 0) out vec4 screen_colour;

void main()
{
    screen_colour = vec4(Frgb, 1.0);
}

