#version 460
#extension GL_EXT_shader_16bit_storage: enable
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
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
	vec2     pos;
	uint8_t  dir, framec;
	uint16_t gi, timer, duration;
};

layout(binding = 10) readonly buffer SpriteSheetBufferSBO {
	int w, h, z;
	int scale;
	SpriteFrame frames[];
} sheet;
layout(binding = 11) readonly buffer SpriteBufferSBO {
	Sprite data[];
} sprites;

layout(push_constant) uniform PushConstants {
	int sheet;
} push;

void main()
{
	Sprite      sprite = sprites.data[gl_VertexIndex / 6];
	SpriteFrame frame  = sheet.frames[sprite.gi + sprite.timer/sprite.duration];
	vec3 pos = rect_verts[gl_VertexIndex % 6];

	Fpos = pos + vec3(sprite.pos, sheet.z);
	Fuv  = vec2(frame.x + frame.w*pos.x, frame.y + frame.h*pos.y) / vec2(sheet.w, sheet.h);

	pos *= vec3(frame.w, frame.h, 0.0f) / (2.0f * sheet.scale);
	pos.z = pos.y + sheet.z;
	pos += vec3((sprite.pos.x - sprite.pos.y) / 2.0f,
	            (sprite.pos.x + sprite.pos.y) / 4.0f,
	           -(sprite.pos.x + sprite.pos.y) / 2.0f);
	gl_Position = cam.proj * cam.view * vec4(pos, 1.0f);
}

