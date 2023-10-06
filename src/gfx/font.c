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
	uint  advance;
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
	0.0, 0.0, 0.0, 0.0,   1.0, 0.0, 1.0, 0.0,   0.0, 1.0, 0.0, 1.0,
    0.0, 1.0, 0.0, 1.0,   1.0, 0.0, 1.0, 0.0,   1.0, 1.0, 1.0, 1.0,
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

	/* Over-allocate - probably a better way of doing this */
	// uint32* pxs  = smalloc(sizeof(uint32)*font_size*(128*2*font_size));
	// uint8* pxs  = smalloc(16*16);
	uint maxh   = 0;
	uint totalw = 0;
	uint tex_x  = 0; /* Current position on the main bitmap */
	FT_GlyphSlot glyph = face->glyph;
	// for (int i = 0; i < 128; i++) {
	// 	if (FT_Load_Char(face, (char)i, FT_LOAD_RENDER))
	// 		ERROR("[GFX] Error loading glyph \"%c\"", (char)i);

	// 	characters[i] = (struct Character){
	// 		.size[0]    = glyph->bitmap.width,
	// 		.size[1]    = glyph->bitmap.rows,
	// 		.bearing[0] = glyph->bitmap_left,
	// 		.bearing[1] = glyph->bitmap_top,
	// 		.advance    = glyph->advance.x,
	// 	};
	// 	if (maxh < glyph->bitmap.rows)
	// 		maxh = glyph->bitmap.rows;
	// 	totalw += glyph->bitmap.width + (glyph->advance.x >> 6);

	// 	/* Copy over the bitmap */
	// 	for (uint y = 0; y < glyph->bitmap.rows; y++) {
	// 		for (uint x = 0; x < glyph->bitmap.width; x++) {
	// 			// DEBUG(1, "%d, %d -> %d - %d [%X]", x, y, y*totalw + x, y*characters[i].size[0] + x, ((uint32*)glyph->bitmap.buffer)[y*characters[i].size[0] + x]);
	// 			pxs[y*totalw + x + tex_x] = ((uint32*)glyph->bitmap.buffer)[y*glyph->bitmap.width + x];
	// 		}
	// 	}
	// 	tex_x += glyph->bitmap.width;
	// }
	// char_texture = texture_new(pxs, totalw, maxh);
	// char_texture = texture_new_from_image("data/img.jpg");

	FT_Load_Char(face, 'X', FT_LOAD_RENDER);
	FT_Bitmap bitmap = face->glyph->bitmap;
	uint8* pxs = scalloc(sizeof(uint8[4]), bitmap.width*bitmap.rows);
    for (uint y = 0; y < bitmap.rows; y++) {
        for (uint x = 0; x < bitmap.width; x++) {
            int index = y*bitmap.width + x;
        	memset(pxs + 4*index, bitmap.buffer[index], sizeof(uint8[4]));
        }
    }
	char_texture = texture_new(pxs, bitmap.width, bitmap.rows);

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
