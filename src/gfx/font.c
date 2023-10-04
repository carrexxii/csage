#include <ft2build.h>
#include FT_FREETYPE_H

#include "font.h"

int font_size = 16;

static FT_Library library;
static FT_Face    face;

void font_init()
{
	if (!library && FT_Init_FreeType(&library))
		ERROR("[GFX] Failed to initialize FreeType");
	if (!face && FT_New_Face(library, FONT_PATH, 0, &face))
		ERROR("[GFX] Failed to load font (\"%s\")", FONT_PATH);

	FT_Set_Pixel_Sizes(face, font_size, font_size);

	DEBUG(2, "[GFX] Font successfully initialized with size %d (%ld glyphs)", font_size, face->num_glyphs);
	exit(0);
}

void font_render(void* surface, int textlen, char* text, int x, int y)
{
	if (!textlen)
		textlen = strlen(text);

	FT_GlyphSlot glyph = face->glyph;
	for (int i = 0; i < textlen; i++) {
		if (FT_Load_Glyph(face, FT_Get_Char_Index(face, text[i]), FT_LOAD_DEFAULT))
			ERROR("[GFX] Error loading glyph \"%c\"", text[i]);

		// my_draw_bitmap(&glyph->bitmap, x + glyph->bitmap_left, y - glyph->bitmap_top);

		x += glyph->advance.x >> 6;
		y += glyph->advance.y >> 6;
	}
}
