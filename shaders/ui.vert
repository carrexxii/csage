#version 460
#extension GL_EXT_shader_16bit_storage: enable
#extension GL_EXT_shader_explicit_arithmetic_types_int8: enable

#include "common.glsl"

layout(location = 0) out vec2 Fpos;
layout(location = 1) out vec4 Fcolour;
layout(location = 2) out flat int Ftex_id;
layout(location = 3) out vec2 Fuv;
layout(location = 4) out vec4 Fhl;

struct UIObject {
	vec4 rect;
	vec4 hl;
	vec4 colour;
	int  tex_id;
};
layout(binding = 10) readonly buffer ObjectsSBO {
	UIObject data[];
} objects;

void main()
{
	int i = gl_VertexIndex / 6;
	UIObject obj = objects.data[i];

	vec2 pos = obj.rect.xy + obj.rect.zw*rect_verts[gl_VertexIndex % 6].xy;
	float z = 0.01f - 0.0001f*i;

	Fpos    = pos;
	Fuv     = rect_verts[gl_VertexIndex % 6].xy;
	Ftex_id = obj.tex_id;
	Fhl     = obj.hl;
	Fcolour = obj.colour;

	gl_Position = vec4(pos, z, 1.0f);
}
