#version 460

layout(location = 0) in vec3 Fnnn;

layout(location = 0) out vec4 rgba;

void main()
{
    rgba = vec4(Fnnn, 1.0);
}
