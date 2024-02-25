#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <vulkan/vulkan.h>

#include "config.h"
#include "util/string.h"
#include "util/varray.h"
#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "gfx/image.h"
#include "gfx/texture.h"
#include "font.h"

struct Character {
	int sz[2];
	int bearing[2];
	int advance;
	int offset; /* In the main texture */
};

/* -------------------------------------------------------------------- */
#define SIZEOF_FONT_VERTEX sizeof(float[4])

static struct Pipeline pipeln;
static VkVertexInputBindingDescription vert_binds[] = {
	{ .binding   = 0,
	  .stride    = SIZEOF_FONT_VERTEX, /* xyuv */
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, }
};
static VkVertexInputAttributeDescription vert_attrs[] = {
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

int font_size = 36;

static FT_Library library;
static FT_Face    face;
static VkSampler  font_sampler;
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
	if (!face && FT_New_Face(library, FONT_PATH "fantasque.ttf", 0, &face))
		ERROR("[GFX] Failed to load font (\"%s\")", FONT_PATH "fantasque.ttf");

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

	DEBUG(2, "[GFX] Font initialized with size %d (%ld available glyphs)", font_size, face->num_glyphs);
	free(atlas_bitmap);
	FT_Done_Face(face);
	FT_Done_FreeType(library);

	font_sampler = image_new_sampler(VK_FILTER_LINEAR);
	pipeln = (struct Pipeline){
		.vshader     = create_shader(SHADER_DIR "font.vert"),
		.fshader     = create_shader(SHADER_DIR "font.frag"),
		.topology    = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.vert_bindc  = 1,
		.vert_binds  = vert_binds,
		.vert_attrc  = 2,
		.vert_attrs  = vert_attrs,
		.push_stages = VK_SHADER_STAGE_VERTEX_BIT,
		.push_sz     = sizeof(float[4]), /* [z_lvl][padding][position to draw] */
		.sampler     = font_sampler,
		.imgc        = 1,
		.dset_cap    = 1,
	};
	pipeln_alloc_dsets(&pipeln);
	pipeln_create_dset(&pipeln, 0, NULL, 0, NULL, 1, &atlas.image_view);
	pipeln_init(&pipeln, renderpass);
}

struct TextObject* font_render(char* text, isize text_len, float z, float w)
{
	if (text_len <= 0)
		text_len = strlen(text);

	struct TextObject* obj = &text_objs[text_objc++];
	float* verts = smalloc(6*SIZEOF_FONT_VERTEX*text_len);
	float* v = verts;
	float x = 0.0f;
	float y = 0.0f;
	float offset, sz[2];
	float char_x, char_y;
	char c;
	for (int i = 0; i < text_len; i++) {
		c = text[i];
		offset = characters[(int)c].offset;
		sz[0]  = characters[(int)c].sz[0];
		sz[1]  = characters[(int)c].sz[1];

		if (x*global_config.winw/2.0f + sz[0] > w) {
			x  = 0.0f;
			y -= atlas_h / global_config.winh;
		}

		char_x = x + (float)characters[(int)c].bearing[0]/global_config.winw;
		char_y = y + ((float)characters[(int)c].bearing[1] - sz[1])/global_config.winh;

		*v++ = char_x;
		*v++ = char_y;
		*v++ = offset/atlas_w;
		*v++ = sz[1]/atlas_h;

		*v++ = char_x + sz[0]/global_config.winw;
		*v++ = char_y;
		*v++ = (offset + sz[0])/atlas_w;
		*v++ = sz[1]/atlas_h;

		*v++ = char_x;
		*v++ = char_y + sz[1]/global_config.winh;
		*v++ = offset/atlas_w;
		*v++ = 0.0;

		*v++ = char_x;
		*v++ = char_y + sz[1]/global_config.winh;
		*v++ = offset/atlas_w;
		*v++ = 0.0;

		*v++ = char_x + sz[0]/global_config.winw;
		*v++ = char_y;
		*v++ = (offset + sz[0])/atlas_w;
		*v++ = sz[1]/atlas_h;

		*v++ = char_x + sz[0]/global_config.winw;
		*v++ = char_y + sz[1]/global_config.winh;
		*v++ = (offset + sz[0])/atlas_w;
		*v++ = 0.0;

		x += (float)(characters[(int)c].advance >> 6)/global_config.winw;
	}
	obj->rect.w = x;
	obj->rect.h = atlas_h / global_config.winh;

	obj->z_lvl  = z;
	obj->active = true;
	obj->vertc  = 6*text_len;
	obj->vbo    = vbo_new(6*text_len*SIZEOF_FONT_VERTEX, verts, true);

	DEBUG(4, "[GFX] Created new text object (%zd characters) for \"%s\" at (%.2f, %.2f, %.2f) [%.2f, %.2f]",
	      text_len, text, x, y, z, obj->rect.w, obj->rect.h);
	free(verts);
	return obj;
}

void font_record_commands(VkCommandBuffer cmd_buf, struct Camera* cam)
{
	(void)cam;
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, pipeln.dsets, 0, NULL);
	for (int i = 0; i < text_objc; i++) {
		if (!text_objs[i].active)
			continue;
		vkCmdBindVertexBuffers(cmd_buf, 0, 1, &text_objs[i].vbo.buf, (VkDeviceSize[]) { 0 });
		vkCmdPushConstants(cmd_buf, pipeln.layout, pipeln.push_stages, 0, pipeln.push_sz, (float[]){
			text_objs[i].z_lvl, 0.0f, text_objs[i].rect.x, text_objs[i].rect.y });
		vkCmdDraw(cmd_buf, text_objs[i].vertc, 1, 0, i);
	}
}

void font_free()
{
	for (int i = 0; i < text_objc; i++)
		vbo_free(&text_objs[i].vbo);
	texture_free(&atlas);
	pipeln_free(&pipeln);
	vkDestroySampler(logical_gpu, font_sampler, NULL);
}
