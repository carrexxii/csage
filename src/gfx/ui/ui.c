#include "vulkan/vulkan.h"

#include "resmgr.h"
#include "input.h"
#include "util/varray.h"
#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "types.h"
#include "ui.h"

static void init_pipeln(void);
static inline void update_container(struct UIContainer* container);
static void cb_mouse_left(bool kdown);

static struct Pipeline pipeln;

int containerc, img_viewc;
static struct UIContainer containers[UI_MAX_CONTAINERS];
static VkImageView img_views[UI_MAX_IMAGES];
static struct UIContext context;

static int  ui_elemc;
static SBO  ui_elems;
static bool ui_elems_update;

static bool pipeln_needs_update;
static struct Texture* default_tex;

void ui_init()
{
	pipeln_needs_update = true;
	ui_elems = sbo_new(UI_MAX_ELEMENTS * sizeof(struct UIShaderObject));
	default_tex = load_texture(STRING(TEXTURE_PATH "/default.png"));

	DEBUG(1, "[UI] Initialized UI");
}

void ui_register_keys()
{
	input_register(SDL_BUTTON_LEFT, cb_mouse_left);
}

struct UIContainer* ui_new_container(Rect rect, struct UIStyle* style)
{
	if (containerc >= UI_MAX_CONTAINERS) {
		ERROR("[UI] Total container count (%d) exceeded", UI_MAX_CONTAINERS);
		return NULL;
	}

	struct UIContainer* container = &containers[containerc++];
	*container = (struct UIContainer){
		.rect    = rect,
		.i       = ui_elemc,
		.objects = varray_new(UI_DEFAULT_OBJECTS, sizeof(struct UIObject)),
	};

	buffer_update(ui_elems, sizeof(struct UIShaderObject), &(struct UIShaderObject){
		.rect   = rect,
		.colour = colour_normalized(style? style->normal: default_container_style.normal),
		.hl     = RECT0,
		.tex_id = -1,
	}, ui_elemc * sizeof(struct UIShaderObject));

	ui_elemc++;
	return container;
}

int ui_add(struct UIContainer* container, struct UIObject* obj)
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
	struct UIContainer* container;
	struct UIObject*  obj;
	for (int i = 0; i < containerc; i++) {
		container = &containers[i];
		if (!container->has_update)
			continue;

		for (int j = 0; j < container->objects.len; j++) {
			obj = varray_get(&container->objects, j);
			switch (obj->type) {
			case UI_BUTTON: button_build(obj)     ; break;
			case UI_LABEL : label_build(obj)      ; break;
			case UI_LIST  : uilist_build(obj)     ; break;
			case UI_CUSTOM: obj->custom.build(obj); break;
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
	context.mouse_pos = VEC2(2.0f*(mouse_x / config.winw - 0.5f), 2.0f*(mouse_y / config.winh - 0.5f));
	Vec2 mouse = context.mouse_pos;

	struct UIContainer* container;
	struct UIObject*  obj;
	for (int i = 0; i < containerc; i++) {
		container = &containers[i];
		if (point_in_rect(mouse, container->rect)) {
			for (int j = 0; j < container->objects.len; j++) {
				obj = varray_get(&container->objects, j);
				if (!obj->state.visible || obj->type == UI_LABEL)
					continue;

				if (point_in_rect(mouse, obj->rect)) {
					bool update = obj->state.clicked && !context.mouse_pressed.lmb;
					switch (obj->type) {
					case UI_BUTTON:
						if (update) button_on_click(obj);
						else update = button_on_hover(obj, &context);
						break;
					case UI_LIST:
						if (update) uilist_on_click(obj, &context);
						else update = uilist_on_hover(obj, &context);
						break;
					case UI_CUSTOM:
						if (update) obj->custom.on_click(obj, &context);
						else update = obj->custom.on_hover(obj, &context);
						break;
					default:
						ERROR("[UI] Failed to match UI object of type %d", obj->type);
					}

					if (update)
						ui_update_object(obj);
				} else if (obj->state.hover) {
					obj->state.hover   = false;
					obj->state.clicked = false;
					ui_update_object(obj);
				}
			}
		}
		container->has_update = true;
	}
}

void ui_update_object(struct UIObject* obj)
{
	assert(obj->i >= 0 && obj->i <= ui_elemc);
	if (!obj->style) {
		ERROR("[UI] Object of type %d does not have a style", obj->type);
		obj->style = &default_style;
	}

	Vec4 colour = colour_normalized(obj->state.clicked? obj->style->clicked:
	                                obj->state.hover  ? obj->style->hover  :
	                                                    obj->style->normal);
	buffer_update(ui_elems, sizeof(struct UIShaderObject), &(struct UIShaderObject){
		.rect   = obj->rect,
		.colour = colour,
		.hl     = obj->hl,
		.tex_id = obj->imgi,
	}, obj->i * sizeof(struct UIShaderObject));
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
	struct UIContainer* container;
	struct UIObject*  obj;
	for (int i = 0; i < containerc; i++) {
		container = &containers[i];
		for (int j = 0; j < container->objects.len; j++) {
			obj = varray_get(&container->objects, j);
			if (obj->type == UI_LIST)
				free(obj->uilist.text_objs);
		}
		varray_free(&container->objects);
	}

	pipeln_free(&pipeln);
}

/* -------------------------------------------------------------------- */

static void init_pipeln()
{
	if (pipeln.pipeln)
		pipeln_free(&pipeln);

	pipeln = (struct Pipeline){
		.vshader    = load_shader(STRING(SHADER_PATH "/ui.vert")),
		.fshader    = load_shader(STRING(SHADER_PATH "/ui.frag")),
		.topology   = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.dset_cap   = 1,
		.sboc       = 1,
		.imgc       = img_viewc? img_viewc: 1,
	};
	pipeln_alloc_dsets(&pipeln);
	pipeln_create_dset(&pipeln,
		pipeln.uboc, NULL,
		pipeln.sboc, &ui_elems,
		pipeln.imgc, img_viewc? img_views: &default_tex->image_view);
	pipeln_init(&pipeln);

	pipeln_needs_update = false;
}

static void cb_mouse_left(bool kdown)
{
	context.mouse_pressed.lmb  =  kdown;
	context.mouse_released.lmb = !kdown;
}
