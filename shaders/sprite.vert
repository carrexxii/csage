#version 460
#extension GL_EXT_shader_16bit_storage: enable
#extension GL_EXT_shader_explicit_arithmetic_types_int16: enable

#include "common.glsl"

layout(location = 0) out vec3 Fpos;
layout(location = 1) out vec2 Fuv;

layout(binding = 0) uniform CameraUBO {
	mat4 proj;
	mat4 view;
} cam;

struct SpriteFrame {
	int16_t x, y, w, h;
};
struct Sprite {
	vec2 pos;
	int  id;
};
layout(binding = 10) readonly buffer SpriteSheetBufferUBO {
	// int w, h;
	SpriteFrame frames[];
} sheet;
layout(binding = 11) readonly buffer SpriteBufferSBO {
	Sprite data[];
} sprites;

void main()
{
	vec3 pos = rect_verts[gl_VertexIndex % 6];
	SpriteFrame frame = sheet.frames[sprites.data[gl_VertexIndex / 6].id];

	Fpos = pos;
	Fuv  = pos.xz;

	gl_Position = cam.proj * cam.view * vec4(pos - vec3(0, 0, 1), 1.0f);
}