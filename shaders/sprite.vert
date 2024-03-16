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
	vec3    pos;
	int16_t start, frame;
	int16_t group, state;
};
layout(binding = 10) readonly buffer SpriteSheetBufferUBO {
	// int w, h;
	SpriteFrame frames[];
} sheet;
layout(binding = 11) readonly buffer SpriteBufferSBO {
	Sprite data[];
} sprites;

layout(push_constant) uniform PushConstants {
	int sheet;
	mat4 view;
} push;

void main()
{
	Sprite      sprite = sprites.data[gl_VertexIndex / 6];
	SpriteFrame frame  = sheet.frames[sprite.start + sprite.frame];
	vec3 pos = rect_verts[gl_VertexIndex % 6];

	Fpos = pos;
	Fuv  = vec2(frame.x + frame.w*pos.x,
	            frame.y + frame.h*pos.y) / 256.0f;

	pos += sprite.pos;
	gl_Position = cam.proj * push.view * vec4(pos, 1.0f);
}
