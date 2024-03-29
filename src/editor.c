#include "SDL3/SDL.h"

#include "resmgr.h"
#include "util/file.h"
#include "gfx/ui/ui.h"
#include "gfx/sprite.h"
#include "editor.h"

struct UITileset {
	struct SpriteSheet* sheet;
} tileset;

static void new_ui_tileset(struct UIContainer* parent, Rect rect);
static void ui_tileset_build(struct UIObject* obj);
static void ui_tileset_set_image(String path);
static bool ui_tileset_on_hover(struct UIObject* obj, struct UIContext* context);
static void ui_tileset_on_click(struct UIObject* obj, struct UIContext* context);

/* -------------------------------------------------------------------- */
static void cb_test(int x) { DEBUG(1, "Clicked: %d", x); }

static struct Arena* arena;
static struct VArray sheets;
static String* selected_file;
static struct UIContainer* tset_container;

static void cb_load_tileset(int) {
	if (!selected_file)
		return;

	tileset.sheet = sprite_sheet_new(selected_file->data, 1);

	if (tset_container)
		ui_free_container(tset_container);
	tset_container = ui_new_container(RECT(-1.0f, -1.0f, 1.0f, 1.0f), NULL);

	Rect rect;
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < UI_TILESET_GRID_COLUMNS; x++) {
			rect = RECT((float)x / 4.0f - 1.0f, (float)y / 4.0f - 1.0f, 0.25f, 0.25f);
			button_new(tset_container, rect, STRING(""), tileset.sheet->albedo, NULL, cb_test, y*UI_TILESET_GRID_COLUMNS + x, NULL);
		}
	}

	ui_build();
}

static void cb_select_file(int index) {
	if (index < 0 || index >= sheets.len)
		selected_file = NULL;
	else
		selected_file = varray_get(&sheets, index);
}
void editor_init()
{
	arena  = arena_new(4096, ARENA_RESIZEABLE);
	sheets = varray_new(32, sizeof(String));

	struct UIContainer* sidebar = ui_new_container(RECT(0.4f, -1.0f, 0.6f, 2.0f), NULL);
	button_new(sidebar, RECT(-0.9f, 0.82f, 0.8f, 0.12f), STRING("Load Tileset"), NULL, NULL, cb_load_tileset, 0, NULL);
	button_new(sidebar, RECT( 0.1f, 0.82f, 0.8f, 0.12f), STRING("Button 2"), NULL, NULL, cb_test, 2, NULL);

	dir_enumerate(SPRITE_SHEETS_PATH, false, false, STRING(".lua"), &sheets, arena);
	uilist_new(sidebar, sheets.len, (String*)sheets.data, NULL, RECT(-0.9f, -0.95f, 1.8f, 0.6f), true, cb_select_file);

	new_ui_tileset(sidebar, RECT(-0.95f, -0.3f, 1.9f, 1.0f));

	arena_reset(arena);
}

void editor_free()
{
	arena_free(arena);
	varray_free(&sheets);
}

/* -------------------------------------------------------------------- */

static struct UIStyle tileset_style = {
	.normal  = 0xFF5555FF,
	.hover   = 0x55FF55FF,
	.clicked = 0x5555FFFF,
};

static void new_ui_tileset(struct UIContainer* parent, Rect rect)
{
	struct UIObject obj = {
		.type    = UI_CUSTOM,
		.style   = &default_style,
		.rect    = ui_calc_rect(rect, parent),
		.imgi    = -1,
		.padding = VEC2(10.0f / config.winw, 10.0f / config.winh),
		.state   = default_state,
		.custom = {
			.build    = ui_tileset_build,
			.on_hover = ui_tileset_on_hover,
			.on_click = ui_tileset_on_click,
		},
	};
	ui_add(parent, &obj);
}

static void ui_tileset_build(struct UIObject* obj)
{
	assert(obj->type == UI_CUSTOM);

	ui_update_object(obj);
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
