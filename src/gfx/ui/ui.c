#include "vulkan/vulkan.h"

#include "input.h"
#include "util/varray.h"
#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "types.h"
#include "ui.h"

static void init_pipeln(void);
static inline void update_container(struct Container* container);
static void cb_mouse_left(bool kdown);

static struct Pipeline pipeln;

int containerc, img_viewc;
static struct Container containers[UI_MAX_CONTAINERS];
static VkImageView img_views[UI_MAX_IMAGES];
static struct {
	Vec2 mouse_pos;
	struct { bool lmb, rmb; } mouse_pressed;
	struct { bool lmb, rmb; } mouse_released;
} context;

static int  ui_elemc;
static SBO  ui_elems;
static bool ui_elems_update;

static bool pipeln_needs_update;
static struct Texture default_tex;

void ui_init()
{
	pipeln_needs_update = true;
	ui_elems = sbo_new(UI_MAX_ELEMENTS * sizeof(struct UIShaderObject));
	default_tex = texture_new_from_image(TEXTURE_PATH "/default.png");

	DEBUG(1, "[UI] Initialized UI");
}

void ui_register_keys()
{
	input_register(SDL_BUTTON_LEFT, cb_mouse_left);
}

struct Container* ui_new_container(Rect rect, struct UIStyle* style)
{
	if (containerc >= UI_MAX_CONTAINERS) {
		ERROR("[UI] Total container count (%d) exceeded", UI_MAX_CONTAINERS);
		return NULL;
	}
	style = style? style: &default_container_style;

	ui_update_object(ui_elemc, rect, style->bg, -1, (struct UIState){ .visible = 1 });

	struct Container* container = &containers[containerc++];
	*container = (struct Container){
		.rect    = rect,
		.i       = ui_elemc++,
		.objects = varray_new(UI_DEFAULT_OBJECTS, sizeof(struct UIObject)),
		.styles  = {
			.container = style,
			.button    = &default_button_style,
			.label     = &default_label_style,
		},
	};

	return container;
}

int ui_add(struct Container* container, struct UIObject* obj)
{
	assert(container && obj);

	obj->i = ui_elemc;
	varray_push(&container->objects, obj);
	container->has_update = true;

	return ui_elemc++;
}

int ui_add_image(VkImageView img_view)
{
	assert(img_view);

	img_views[img_viewc] = img_view;

	return img_viewc++;
}

void ui_build()
{
	struct Container* container;
	struct UIObject*  obj;
	for (int i = 0; i < containerc; i++) {
		container = &containers[i];
		if (!container->has_update)
			continue;

		for (int j = 0; j < container->objects.len; j++) {
			obj = varray_get(&container->objects, j);
			switch (obj->type) {
			case UI_BUTTON: button_build(obj, container->styles.button); break;
			case UI_LABEL : label_build(obj,  container->styles.label) ; break;
			default:
				ERROR("[UI] Failed to build object with type %d", obj->type);
			}
		}
	}

	if (pipeln_needs_update)
		init_pipeln();
}

void ui_update()
{
	float mx = ((float)mouse_x / config.winw - 0.5f)*2.0f;
	float my = ((float)mouse_y / config.winh - 0.5f)*2.0f;
	Vec2 mouse = VEC2(2.0f*(mouse_x / config.winw - 0.5f), 2.0f*(mouse_y / config.winh - 0.5f));

	struct Container* container;
	struct UIObject*  obj;
	for (int i = 0; i < containerc; i++) {
		container = &containers[i];
		if (point_in_rect(mouse, container->rect)) {
			for (int j = 0; j < container->objects.len; j++) {
				obj = varray_get(&container->objects, j);
				if (obj->type != UI_BUTTON)
					continue;

				if (point_in_rect(mouse, obj->rect)) {
					if (obj->type == UI_BUTTON && obj->state.clicked && !context.mouse_pressed.lmb)
						button_on_click(obj);

					obj->state.hover   = true;
					obj->state.clicked = context.mouse_pressed.lmb;
					ui_update_object(obj->i, obj->rect, container->styles.button->bg, obj->imgi, obj->state);
				} else if (obj->state.hover) {
					obj->state.hover   = false;
					obj->state.clicked = false;
					ui_update_object(obj->i, obj->rect, container->styles.button->bg, obj->imgi, obj->state);
				}
			}
		}
		container->has_update = true;
	}
}

void ui_update_object(int i, Rect rect, Colour colour, int tex_id, struct UIState state)
{
	assert(i >= 0 && i <= ui_elemc);

	buffer_update(ui_elems, sizeof(struct UIShaderObject), &(struct UIShaderObject){
		.rect   = rect,
		.colour = colour_normalized(colour),
		.tex_id = tex_id,
		.state  = state,
	}, i * sizeof(struct UIShaderObject));
}

void ui_record_commands(VkCommandBuffer cmd_buf)
{
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, pipeln.dsets, 0, NULL);

	int objc = containerc;
	for (int i = 0; i < containerc; i++)
		objc += containers[i].objects.len;
	vkCmdDraw(cmd_buf, 6*objc, 1, 0, 0);
}

void ui_free()
{
	for (int i = 0; i < containerc; i++)
		varray_free(&containers[i].objects);

	texture_free(&default_tex);
	pipeln_free(&pipeln);
}

/* -------------------------------------------------------------------- */

static void init_pipeln()
{
	if (pipeln.pipeln)
		pipeln_free(&pipeln);

	pipeln = (struct Pipeline){
		.vshader    = create_shader(SHADER_PATH "/ui.vert"),
		.fshader    = create_shader(SHADER_PATH "/ui.frag"),
		.topology   = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.dset_cap   = 1,
		.sboc       = 1,
		.imgc       = img_viewc? img_viewc: 1,
	};
	pipeln_alloc_dsets(&pipeln);
	pipeln_create_dset(&pipeln,
		pipeln.uboc, NULL,
		pipeln.sboc, &ui_elems,
		pipeln.imgc, img_viewc? img_views: &default_tex.image_view);
	pipeln_init(&pipeln);

	pipeln_needs_update = false;
}

static void cb_mouse_left(bool kdown)
{
	context.mouse_pressed.lmb  =  kdown;
	context.mouse_released.lmb = !kdown;
}
