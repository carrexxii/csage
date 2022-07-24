#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec3 vert_colour;
layout(location = 1) out vec3 vert_normal;

layout(binding = 0) uniform UniformBufferObject {
    mat4 vp;
} cam;

void main() {
    gl_Position = cam.vp * vec4(pos, 1.0);
    vert_colour  = colour;
    vert_normal  = normal;

    /* glm (OpenGL) -> Vulkan depth */
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}
