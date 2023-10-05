#include "common.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "vulkan/vulkan.h"

#include "vulkan.h"
#include "texture.h"
#include "font.h"

struct Character {
	void* texture;
	uint8 sz[2];
	uint8 bearing[2];
	uint  advance;
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
static struct Texture chartexture;

void font_init(VkRenderPass renderpass)
{
	pipeln = (struct Pipeline){
		.vshader   = create_shader(SHADER_DIR "font.vert"),
		.fshader   = create_shader(SHADER_DIR "font.frag"),
		.vertbindc = 1,
		.vertbinds = &fontvertbind,
		.vertattrc = 2,
		.vertattrs = fontvertattrs,
	};
	init_pipeln(&pipeln, renderpass);

	if (!library && FT_Init_FreeType(&library))
		ERROR("[GFX] Failed to initialize FreeType");
	if (!face && FT_New_Face(library, FONT_PATH, 0, &face))
		ERROR("[GFX] Failed to load font (\"%s\")", FONT_PATH);

	FT_Set_Pixel_Sizes(face, font_size, font_size);

	uint maxh   = 0;
	uint totalw = 0;
	FT_GlyphSlot glyph = face->glyph;
	for (int i = 0; i < 128; i++) {
		if (FT_Load_Char(face, (char)i, FT_LOAD_RENDER))
			ERROR("[GFX] Error loading glyph \"%c\"", (char)i);

		characters[i] = (struct Character){
			.sz[0]      = glyph->bitmap.width,
			.sz[1]      = glyph->bitmap.rows,
			.bearing[0] = glyph->bitmap_left,
			.bearing[1] = glyph->bitmap_top,
			.advance    = glyph->advance.x,
		};
		if (maxh < glyph->bitmap.rows)
			maxh = glyph->bitmap.rows;
		totalw += glyph->bitmap.width + (glyph->advance.x >> 6);
	}
	// DEBUG_VALUE(maxh);
	// DEBUG_VALUE(totalw);
	uint8* pxs = smalloc(4*totalw*maxh);
	chartexture = texture_new(pxs, totalw, maxh);

	FT_Done_Face(face);
	FT_Done_FreeType(library);
	DEBUG(2, "[GFX] Font initialized with size %d (%ld available glyphs)", font_size, face->num_glyphs);
}

VBO font_render(char* text, float x, float y, float maxw, float maxh)
{
	VBO vbo;
	for (char c = *text; c; c = *++text) {
		DEBUG_VALUE(c);
	}

	return vbo;
}

void font_free()
{
	texture_free(chartexture);
	pipeln_free(&pipeln);
}
