#version 450

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_colour;

layout(location = 0) out vec3 out_colour;

layout(binding = 0) uniform UniformBufferObject {
    mat4 vp;
} cam;

void main() {
    gl_Position = cam.vp * vec4(in_pos, 1.0);
    out_colour  = in_colour;

    /* glm (OpenGL) -> Vulkan depth */
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}
