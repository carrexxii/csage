#include "common.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include "vulkan/vulkan.h"

#include "util/varray.h"
#include "vulkan.h"
#include "texture.h"
#include "font.h"

struct Character {
	uint8 size[2];
	uint8 bearing[2];
	uint8 advance;
	uint  offset; /* In the main texture */
};

struct TextObject {
	VBO  vbo;
	bool active;
};

/* -------------------------------------------------------------------- */
#define SIZEOF_FONT_VERTEX sizeof(float[4])

struct Pipeline pipeln;

static VkVertexInputBindingDescription fontvertbind = {
	.binding   = 0,
	.stride    = SIZEOF_FONT_VERTEX, /* xyuv */
	.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
};
static VkVertexInputAttributeDescription fontvertattrs[] = {
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

int font_size = 16;

static FT_Library library;
static FT_Face    face;
static struct Character characters[128];
static struct Texture char_texture;
static struct TextObject text_objs[FONT_MAX_TEXT_OBJECTS];
static intptr text_objc;


float* verts = (float[24]){
	-0.9, 0.0, 0.0, 0.0,   0.9, 0.0, 1.0, 0.0,   -0.9, 0.1, 0.0, 1.0,
    -0.9, 0.1, 0.0, 1.0,   0.9, 0.0, 1.0, 0.0,    0.9, 0.1, 1.0, 1.0,
};
VBO vbo;

void font_init(VkRenderPass renderpass)
{
	vbo = vbo_new(sizeof(float[24]), verts);

	if (!library && FT_Init_FreeType(&library))
		ERROR("[GFX] Failed to initialize FreeType");
	if (!face && FT_New_Face(library, FONT_PATH, 0, &face))
		ERROR("[GFX] Failed to load font (\"%s\")", FONT_PATH);

	FT_Set_Pixel_Sizes(face, font_size, font_size);

	/* Calculate the sizes needed for the atlas */
	uint atlas_w = 0;
	uint atlas_h = 0;
	FT_Bitmap bitmap;
	for (int i = 0; i < 128; i++) {
		if (FT_Load_Char(face, (char)i, FT_LOAD_NO_BITMAP))
			ERROR("[GFX] Error loading glyph data \"%c\"", (char)i);

		bitmap = face->glyph->bitmap;
		if (atlas_h < bitmap.rows)
			atlas_h = bitmap.rows;
		atlas_w += face->glyph->advance.x >> 6;
	}

	uint8* atlas = scalloc(sizeof(uint8[4]), sizeof(uint8[4])*atlas_w*atlas_h);
	uint tex_x   = 0; /* Current position on the main bitmap */
	int atlas_i, bitmap_i;
	for (int i = 0; i < 128; i++) {
		if (FT_Load_Char(face, (char)i, FT_LOAD_RENDER))
			ERROR("[GFX] Error loading glyph bitmap \"%c\"", (char)i);

		bitmap = face->glyph->bitmap;
		characters[i] = (struct Character){
			.size[0]    = bitmap.width,
			.size[1]    = bitmap.rows,
			.bearing[0] = face->glyph->bitmap_left,
			.bearing[1] = face->glyph->bitmap_top,
			.advance    = face->glyph->advance.x,
			.offset     = tex_x,
		};

		for (uint y = 0; y < bitmap.rows; y++) {
			for (uint x = 0; x < bitmap.width; x++) {
				atlas_i  = y*atlas_w + tex_x + x;
				bitmap_i = y*bitmap.width + x;
				atlas[4*atlas_i + 0] = bitmap.buffer[bitmap_i];
				atlas[4*atlas_i + 1] = bitmap.buffer[bitmap_i];
				atlas[4*atlas_i + 2] = bitmap.buffer[bitmap_i];
				atlas[4*atlas_i + 3] = bitmap.buffer[bitmap_i];
			}
		}
		tex_x += face->glyph->advance.x >> 6;
	}
	char_texture = texture_new(atlas, atlas_w, atlas_h);

	FT_Done_Face(face);
	FT_Done_FreeType(library);
	DEBUG(2, "[GFX] Font initialized with size %d (%ld available glyphs)", font_size, face->num_glyphs);

	pipeln = (struct Pipeline){
		.vshader   = create_shader(SHADER_DIR "font.vert"),
		.fshader   = create_shader(SHADER_DIR "font.frag"),
		.vertbindc = 1,
		.vertbinds = &fontvertbind,
		.vertattrc = 2,
		.vertattrs = fontvertattrs,
		.texturec  = 1,
		.textures  = &char_texture,
	};
	init_pipeln(&pipeln, renderpass);
}

int font_render(char* text, float x, float y, float maxw, float maxh)
{
	// VBO vbo;
	// for (char c = *text; c; c = *++text) {
	// 	DEBUG_VALUE(c);
	// }

	return 0;
}

void font_record_commands(VkCommandBuffer cmdbuf)
{

	vkCmdBindPipeline(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindDescriptorSets(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, &pipeln.dset, 0, NULL);
	// for (int i = 0; i < text_objc; i++) {
	// 	if (!text_objs[i].active)
	// 		continue;
	// 	vkCmdBindVertexBuffers(cmdbuf, 0, 1, &text_objs[i].vbo.buf, (VkDeviceSize[]) { 0 });
	// 	vkCmdDraw(cmdbuf, 6, 1, 0, i);
	// }

	vkCmdBindVertexBuffers(cmdbuf, 0, 1, &vbo.buf, (VkDeviceSize[]) { 0 });
	vkCmdDraw(cmdbuf, 6, 1, 0, 0);
}

void font_free()
{
	texture_free(char_texture);
	pipeln_free(&pipeln);
}
