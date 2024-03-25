#include "SDL3/SDL.h"

#include "util/file.h"
#include "gfx/ui/ui.h"
#include "editor.h"

static struct Arena* arena;

static void cb_test() { DEBUG_VALUE("Clicked"); }
void editor_init()
{
	arena = arena_new(4096, ARENA_RESIZEABLE);

	struct Container* sidebar = ui_new_container(RECT(0.4f, -1.0f, 0.6f, 2.0f), NULL);
	button_new(sidebar, STRING("Button 1"), NULL, RECT(-0.9f, 0.82f, 0.8f, 0.12f), cb_test);
	button_new(sidebar, STRING("Button 2"), NULL, RECT( 0.1f, 0.82f, 0.8f, 0.12f), cb_test);

	struct VArray sheets = varray_new(32, sizeof(String));
	dir_enumerate(SPRITE_PATH "/sheets", &sheets, arena);

	uilist_new(sidebar, sheets.len, (String*)sheets.data, RECT(-0.9f, -0.9f, 1.8f, 1.0f), true);
	// String str = string_new_join(sheets.len, (String*)sheets.data, STRING("\n"), arena);
	// label_new(sidebar, str, RECT(-0.9f, -0.8f, 1.8f, 0.5f));

	arena_reset(arena);
}

void editor_free()
{
	arena_free(arena);
}
