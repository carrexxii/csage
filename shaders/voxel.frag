#version 460

layout(location = 0) in vec3 Fxyz;
layout(location = 1) in vec3 Fnormal;

layout(location = 0) out vec4 screen_colour;

struct MapData {
    ivec3 dim;
    ivec3 stride;
    ivec4 selection[2]; // Top left and bottom right of selection
    int   zlvl;
};
layout(std140, binding = 0) readonly buffer ObjectBuffer {
    MapData data;
} map;

layout(binding = 2) uniform UniformBufferObject {
    vec3  sundir;
    float ambient;
    float sunpower;
    vec4  lights[];
} lighting;

void main()
{
    vec3 diffuse = vec3(max(dot(Fnormal, lighting.sundir), 0.0) * lighting.sunpower);
    diffuse += vec3(max(dot(Fnormal, normalize(vec3(lighting.lights[0]))), 0.0) * lighting.lights[0].w);
    screen_colour = vec4(abs(Fnormal)/3.0 * (lighting.ambient + diffuse), 1.0);

    // Selection highlighting
    // There should be a much better way to do this (without branch?)
    vec3 p1 = vec3(map.data.selection[0]);
    vec3 p2 = vec3(map.data.selection[1]);
    if (Fxyz.z == p1.z && Fxyz.z == p2.z && 
        Fxyz.x <= max(p1.x, p2.x) + 1.0 && Fxyz.x >= min(p1.x, p2.x) &&
        Fxyz.y <= max(p1.y, p2.y) + 1.0 && Fxyz.y >= min(p1.y, p2.y)) {
        screen_colour.xyz += vec3(0.1, 0.1, 0.1);
    }
}

