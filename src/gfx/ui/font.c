#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include "vulkan/vulkan.h"

#include "resmgr.h"
#include "config.h"
#include "util/string.h"
#include "util/varray.h"
#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "gfx/image.h"
#include "font.h"

struct Character {
	int sz[2];
	int bearing[2];
	int advance;
	int offset; /* In the main texture */
};

/* -------------------------------------------------------------------- */
#define SIZEOF_FONT_VERTEX sizeof(float[4])

static struct Pipeline* pipeln;
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
static struct Image atlas;
static float atlas_w;
static float atlas_h;
static int text_objc;
static struct TextObject text_objs[FONT_MAX_TEXT_OBJECTS];

void font_init()
{
	char path[PATH_BUFFER_SIZE];
	snprintf(path, PATH_BUFFER_SIZE, FONT_PATH "/%s.ttf", config.font_name);
	if (!library && FT_Init_FreeType(&library))
		ERROR("[GFX] Failed to initialize FreeType");
	if (!face && FT_New_Face(library, path, 0, &face))
		ERROR("[GFX] Failed to load font (\"%s\")", path);

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
	atlas = image_of_memory(atlas_w, atlas_h, atlas_bitmap);

	DEBUG(2, "[GFX] Font initialized with size %d (%ld available glyphs)", font_size, face->num_glyphs);
	free(atlas_bitmap);
	FT_Done_Face(face);
	FT_Done_FreeType(library);

	font_sampler = image_new_sampler(VK_FILTER_LINEAR);
	struct PipelineCreateInfo pipeln_ci = {
		.vshader     = STRING(SHADER_PATH "/font.vert"),
		.fshader     = STRING(SHADER_PATH "/font.frag"),
		.vert_bindc  = ARRAY_SIZE(vert_binds),
		.vert_binds  = vert_binds,
		.vert_attrc  = ARRAY_SIZE(vert_attrs),
		.vert_attrs  = vert_attrs,
		.push_stages = VK_SHADER_STAGE_VERTEX_BIT,
		.push_sz     = sizeof(float[4]), /* [z_lvl][padding][position to draw] */
		.sampler     = font_sampler,
		.imgc        = 1,
		.imgs        = &atlas,
	};
	pipeln = pipeln_new(&pipeln_ci);
	pipeln_update(pipeln);
}

struct TextObject* font_render(String str, float z, float w)
{
	assert(str.len && str.data);

	struct TextObject* obj = &text_objs[text_objc];
	float* verts = smalloc(6*SIZEOF_FONT_VERTEX*str.len);
	float* v = verts;
	int linec = 1;
	float x = 0.0f;
	float y = 0.0f;
	float offset, sz[2];
	float char_x, char_y;
	char c;
	for (int i = 0; i < str.len; i++) {
		c = str.data[i];
		offset = characters[(int)c].offset;
		sz[0]  = characters[(int)c].sz[0];
		sz[1]  = characters[(int)c].sz[1];

		if (c == '\n' || x*config.winw/2.0f + sz[0] > w) {
			linec++;
			x  = 0.0f;
			y -= atlas_h / config.winh;
			if (c == '\n')
				continue;
		}

		char_x = x + (float)characters[(int)c].bearing[0]/config.winw;
		char_y = y + ((float)characters[(int)c].bearing[1] - sz[1])/config.winh;

		*v++ = char_x;
		*v++ = char_y;
		*v++ = offset/atlas_w;
		*v++ = sz[1]/atlas_h;

		*v++ = char_x + sz[0]/config.winw;
		*v++ = char_y;
		*v++ = (offset + sz[0])/atlas_w;
		*v++ = sz[1]/atlas_h;

		*v++ = char_x;
		*v++ = char_y + sz[1]/config.winh;
		*v++ = offset/atlas_w;
		*v++ = 0.0;

		*v++ = char_x;
		*v++ = char_y + sz[1]/config.winh;
		*v++ = offset/atlas_w;
		*v++ = 0.0;

		*v++ = char_x + sz[0]/config.winw;
		*v++ = char_y;
		*v++ = (offset + sz[0])/atlas_w;
		*v++ = sz[1]/atlas_h;

		*v++ = char_x + sz[0]/config.winw;
		*v++ = char_y + sz[1]/config.winh;
		*v++ = (offset + sz[0])/atlas_w;
		*v++ = 0.0;

		x += (float)(characters[(int)c].advance >> 6)/config.winw;
	}
	obj->rect.w = x;
	obj->rect.h = atlas_h / config.winh * linec;

	obj->z_lvl  = z;
	obj->active = true;
	obj->vertc  = 6*str.len;
	obj->vbo = vbo_new(6*str.len*SIZEOF_FONT_VERTEX, verts, true);
	text_objc++;

	DEBUG(4, "[GFX] Created new text object (%zd characters) for \"%s\" at (%.2f, %.2f, %.2f) [%.2fx%.2f]",
	      str.len, str.data, x, y, z, obj->rect.w, obj->rect.h);
	free(verts);
	return obj;
}

void font_record_commands(VkCommandBuffer cmd_buf)
{
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln->pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln->layout, 0, 1, &pipeln->dset.set, 0, NULL);
	for (int i = 0; i < text_objc; i++) {
		if (!text_objs[i].active)
			continue;

		vkCmdBindVertexBuffers(cmd_buf, 0, 1, &text_objs[i].vbo.buf, (VkDeviceSize[]) { 0 });
		vkCmdPushConstants(cmd_buf, pipeln->layout, pipeln->push_stages, 0, sizeof(float[4]), (float[]){ // TODO: make struct
			text_objs[i].z_lvl, 0.0f, text_objs[i].rect.x, text_objs[i].rect.y
		});
		vkCmdDraw(cmd_buf, text_objs[i].vertc, 1, 0, i);
	}
}

void font_free()
{
	for (int i = 0; i < text_objc; i++)
		vbo_free(&text_objs[i].vbo);
	image_free(&atlas);
	pipeln_free(pipeln);
	vkDestroySampler(logical_gpu, font_sampler, NULL);
}
