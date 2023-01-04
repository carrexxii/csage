#version 460

layout(location = 0) in vec3 Fxyz;
layout(location = 1) in vec3 Fnormal;

layout(location = 0) out vec4 screen_colour;

struct MapData {
    ivec3 dim;
    ivec3 stride;
    ivec4 selection; // w-component should indicate if it is active or not
    // vec3 materials[255];
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
    screen_colour = vec4(abs(Fnormal) * (lighting.ambient + diffuse), 1.0);
    if (map.data.selection.w == 1 && map.data.selection.xyz == ivec3(Fxyz)) {
        screen_colour.xyz += vec3(0.1, 0.1, 0.1);
    }
}
