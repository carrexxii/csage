#version 460

layout(location = 0) in vec2 Fuv;

layout(location = 0) out vec4 rgba;

void main()
{
    rgba = vec4(Fuv, 1.0, 1.0);
}
