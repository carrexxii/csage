#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include "vulkan/vulkan.h"

#include "config.h"
#include "util/varray.h"
#include "vulkan.h"
#include "gfx/buffers.h"
#include "texture.h"
#include "font.h"

struct Character {
	int sz[2];
	int bearing[2];
	int advance;
	int offset; /* In the main texture */
};

struct TextObject {
	VBO  vbo;
	uint vertc;
	bool active;
};

/* -------------------------------------------------------------------- */
#define SIZEOF_FONT_VERTEX sizeof(float[4])

static struct Pipeline pipeln;
static UBO ubo_buf;

static VkVertexInputBindingDescription fontvertbind = {
	.binding   = 0,
	.stride    = SIZEOF_FONT_VERTEX, /* xyuv */
	.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
};
static VkVertexInputAttributeDescription fontvert_attrs[] = {
	{ .binding  = 0,
	  .location = 0,
	  .format   = VK_FORMAT_R32G32_SFLOAT, /* xy */
	  .offset   = 0, },
	{ .binding  = 0,
	  .location = 1,
	  .format   = VK_FORMAT_R32G32_SFLOAT, /* uv */
	  .offset   = sizeof(float[2]), }
};
/* -------------------------------------------------------------------- */

int font_size = 24;

static FT_Library library;
static FT_Face    face;
static struct Character characters[128];
static struct Texture atlas;
static float atlas_w;
static float atlas_h;
static struct TextObject text_objs[FONT_MAX_TEXT_OBJECTS];
static intptr text_objc;

void font_init(VkRenderPass renderpass)
{
	if (!library && FT_Init_FreeType(&library))
		ERROR("[GFX] Failed to initialize FreeType");
	if (!face && FT_New_Face(library, FONT_PATH, 0, &face))
		ERROR("[GFX] Failed to load font (\"%s\")", FONT_PATH);

	FT_Set_Pixel_Sizes(face, font_size, font_size);

	/* Calculate the sizes needed for the atlas */
	atlas_w = 0;
	atlas_h = 0;
	FT_Bitmap bitmap;
	for (int i = 0; i < 128; i++) {
		if (FT_Load_Char(face, (char)i, FT_LOAD_NO_BITMAP))
			ERROR("[GFX] Error loading glyph data \"%c\"", (char)i);

		bitmap = face->glyph->bitmap;
		if (atlas_h < bitmap.rows)
			atlas_h = bitmap.rows;
		atlas_w += face->glyph->advance.x >> 6;
	}

	uint8* atlas_bitmap = scalloc(sizeof(uint8[4]), sizeof(uint8[4])*atlas_w*atlas_h);
	uint tex_x = 0; /* Current position on the main bitmap */
	int atlas_i, bitmap_i;
	for (int i = 0; i < 128; i++) {
		if (FT_Load_Char(face, (char)i, FT_LOAD_RENDER))
			ERROR("[GFX] Error loading glyph bitmap \"%c\"", (char)i);

		bitmap = face->glyph->bitmap;
		characters[i] = (struct Character){
			.sz[0]      = bitmap.width,
			.sz[1]      = bitmap.rows,
			.bearing[0] = face->glyph->bitmap_left,
			.bearing[1] = face->glyph->bitmap_top,
			.advance    = face->glyph->advance.x,
			.offset     = tex_x,
		};

		for (uint y = 0; y < bitmap.rows; y++) {
			for (uint x = 0; x < bitmap.width; x++) {
				atlas_i  = y*atlas_w + tex_x + x;
				bitmap_i = y*bitmap.width + x;
				atlas_bitmap[4*atlas_i + 0] = bitmap.buffer[bitmap_i];
				atlas_bitmap[4*atlas_i + 1] = bitmap.buffer[bitmap_i];
				atlas_bitmap[4*atlas_i + 2] = bitmap.buffer[bitmap_i];
				atlas_bitmap[4*atlas_i + 3] = bitmap.buffer[bitmap_i];
			}
		}
		tex_x += face->glyph->advance.x >> 6;
	}
	atlas = texture_new(atlas_bitmap, atlas_w, atlas_h);

	FT_Done_Face(face);
	FT_Done_FreeType(library);
	DEBUG(2, "[GFX] Font initialized with size %d (%ld available glyphs)", font_size, face->num_glyphs);

	ubo_buf = ubo_new(sizeof(mat4));
	pipeln = (struct Pipeline){
		.vshader   = create_shader(SHADER_DIR "font.vert"),
		.fshader   = create_shader(SHADER_DIR "font.frag"),
		.vert_bindc = 1,
		.vert_binds = &fontvertbind,
		.vert_attrc = 2,
		.vert_attrs = fontvert_attrs,
		.texturec  = 1,
		.textures  = &atlas,
		.uboc      = 1,
		.ubos      = &ubo_buf,
		.enable_blending = true,
	};

	pipeln_init(&pipeln, renderpass);

	mat4 proj;
	glm_ortho(0.0, global_config.winw, global_config.winh, 0.0, 0.0, 1.0, proj);
	buffer_update(ubo_buf, sizeof(mat4), proj);
}

int font_render(char* text, float start_x, float start_y, float w)
{
	(void)w; // TODO: width/multi-line rendering
	int len = strlen(text);
	float* verts = smalloc(6*sizeof(float[4])*len);
	float  x = start_x;
	float  y = global_config.winh - start_y;
	float* v = verts;
	float offset, sz[2];
	float char_x, char_y;
	for (char c = *text; c; c = *++text) {
		offset = characters[(int)c].offset;
		sz[0]  = characters[(int)c].sz[0];
		sz[1]  = characters[(int)c].sz[1];

		char_x = x + characters[(int)c].bearing[0];
		char_y = y + characters[(int)c].bearing[1] - sz[1];

		*v++ = char_x;
		*v++ = char_y;
		*v++ = offset/atlas_w;
		*v++ = sz[1]/atlas_h;

		*v++ = char_x + sz[0];
		*v++ = char_y;
		*v++ = (offset + sz[0])/atlas_w;
		*v++ = sz[1]/atlas_h;

		*v++ = char_x;
		*v++ = char_y + sz[1];
		*v++ = offset/atlas_w;
		*v++ = 0.0;

		*v++ = char_x;
		*v++ = char_y + sz[1];
		*v++ = offset/atlas_w;
		*v++ = 0.0;

		*v++ = char_x + sz[0];
		*v++ = char_y;
		*v++ = (offset + sz[0])/atlas_w;
		*v++ = sz[1]/atlas_h;

		*v++ = char_x + sz[0];
		*v++ = char_y + sz[1];
		*v++ = (offset + sz[0])/atlas_w;
		*v++ = 0.0;

		x += characters[(int)c].advance >> 6;
	}

	text_objs[text_objc].active = true;
	text_objs[text_objc].vertc  = 6*len;
	text_objs[text_objc].vbo = vbo_new(text_objs[text_objc].vertc*sizeof(float[4]), verts);

	free(verts);

	return text_objc++;
}

void font_record_commands(VkCommandBuffer cmdbuf)
{
	vkCmdBindPipeline(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindDescriptorSets(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, &pipeln.dset, 0, NULL);
	for (int i = 0; i < text_objc; i++) {
		if (!text_objs[i].active)
			continue;
		vkCmdBindVertexBuffers(cmdbuf, 0, 1, &text_objs[i].vbo.buf, (VkDeviceSize[]) { 0 });
		vkCmdDraw(cmdbuf, text_objs[i].vertc, 1, 0, i);
	}
}

void font_free()
{
	for (int i = 0; i < text_objc; i++)
		vbo_free(&text_objs[i].vbo);
	texture_free(atlas);
	pipeln_free(&pipeln);
}
