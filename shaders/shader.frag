#version 450

layout(location = 0) in vec3 vert_colour;
layout(location = 1) in vec3 vert_normal;

layout(location = 0) out vec4 screen_colour;

void main() {
    // screen_colour = vec4(vert_colour, 1.0);
    screen_colour = vec4(abs(vert_normal), 1.0);
}

