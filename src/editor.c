#include "SDL3/SDL.h"

#include "util/file.h"
#include "gfx/ui/ui.h"
#include "editor.h"

struct UITileset {
	struct Texture* tex;
};

static void new_ui_tileset(struct UIContainer* parent, Rect rect);
static void ui_tileset_set_image(String path);
static bool ui_tileset_on_hover(struct UIObject* obj, struct UIContext* context);
static void ui_tileset_on_click(struct UIObject* obj, struct UIContext* context);

/* -------------------------------------------------------------------- */

static struct Arena* arena;
struct VArray sheets;

static String* selected_file;

static void cb_load_tileset() {
	if (selected_file)
		DEBUG_VALUE(selected_file->data);
}

static void cb_select_file(int index) {
	if (index < 0 || index >= sheets.len)
		selected_file = NULL;
	else
		selected_file = varray_get(&sheets, index);
}
static void cb_test() { DEBUG_VALUE("Clicked"); }
void editor_init()
{
	arena  = arena_new(4096, ARENA_RESIZEABLE);
	sheets = varray_new(32, sizeof(String));

	struct UIContainer* sidebar = ui_new_container(RECT(0.4f, -1.0f, 0.6f, 2.0f), NULL);
	button_new(sidebar, STRING("Load Tileset"), NULL, RECT(-0.9f, 0.82f, 0.8f, 0.12f), cb_load_tileset);
	button_new(sidebar, STRING("Button 2"), NULL, RECT( 0.1f, 0.82f, 0.8f, 0.12f), cb_test);

	dir_enumerate(SPRITE_SHEETS_PATH, false, &sheets, arena);
	uilist_new(sidebar, sheets.len, (String*)sheets.data, RECT(-0.9f, -0.95f, 1.8f, 0.6f), true, cb_select_file);

	new_ui_tileset(sidebar, RECT(-0.95f, -0.3f, 1.9f, 1.0f));

	arena_reset(arena);
}

void editor_free()
{
	arena_free(arena);
	varray_free(&sheets);
}

/* -------------------------------------------------------------------- */

static void new_ui_tileset(struct UIContainer* parent, Rect rect)
{
	struct UIObject obj = {
		.type    = UI_CUSTOM,
		.rect    = ui_calc_rect(rect, parent),
		.imgi    = -1,
		.padding = VEC2(10.0f / config.winw, 10.0f / config.winh),
		.state   = default_state,
		.custom = {
			.on_hover = ui_tileset_on_hover,
			.on_click = ui_tileset_on_click,
		},
	};
	ui_add(parent, &obj);
}

static void ui_tileset_set_image(String path)
{

}

static bool ui_tileset_on_hover(struct UIObject* obj, struct UIContext* context)
{
	bool update = !obj->state.hover;
	obj->state.hover   = true;
	obj->state.clicked = context->mouse_pressed.lmb;

	return update;
}

static void ui_tileset_on_click(struct UIObject* obj, struct UIContext* context)
{
	DEBUG_VALUE("click");
	obj->state.clicked = false;
}
