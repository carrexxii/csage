#include <vulkan/vulkan.h>

#include "resmgr.h"
#include "input.h"
#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "types.h"
#include "ui.h"

static inline void update_container(UIContainer* container);
static void cb_mouse_left(bool kdown);
static void container_free(UIContainer* container);

static PipelineCreateInfo pipeln_ci;
static Pipeline* atomic pipeln;
static Mutex updating_lock;

int containerc;
static UIContainer containers[UI_MAX_CONTAINERS];
static UIContext   context;

static int  ui_elemc;
static SBO  ui_elems;
static bool ui_elems_update;
static VArray free_elems;

static bool pipeln_needs_update;
static Image* default_tex;

void ui_init()
{
	ui_elems    = sbo_new(UI_MAX_ELEMENTS * sizeof(UIShaderObject)); // TODO: resize if necessary
	free_elems  = varray_new(16, sizeof(int));
	default_tex = load_image(STRING(TEXTURE_PATH "/default.png"));

	pipeln_ci = (PipelineCreateInfo){
		.fshader       = STRING(SHADER_PATH "/ui.frag"),
		.vshader       = STRING(SHADER_PATH "/ui.vert"),
		.sbo_stages[0] = VK_SHADER_STAGE_VERTEX_BIT,
		.sboc          = 1,
		.sbos[0]       = ui_elems,
		.imgc          = 1,
		.imgs[0]       = default_tex,
	};
	pipeln = pipeln_new(&pipeln_ci, "UI");
	pipeln = pipeln_update(pipeln, &pipeln_ci);

	INFO(TERM_DARK_CYAN "[UI] Initialized UI");
}

void ui_register_keys()
{
	input_register(SDL_BUTTON_LEFT, cb_mouse_left);
}

UIContainer* ui_new_container(Rect rect, UIStyle* style)
{
	if (containerc >= UI_MAX_CONTAINERS) {
		ERROR("[UI] Total container count (%d) exceeded", UI_MAX_CONTAINERS);
		return NULL;
	}

	int i;
	for (i = 0; i < containerc; i++)
		if (containers[i].state.dead)
			break;
	if (i >= containerc)
		containerc++;

	UIContainer* container = &containers[i];
	*container = (UIContainer){
		.rect    = rect,
		.i       = ui_elemc,
		.objects = varray_new(UI_DEFAULT_OBJECTS, sizeof(UIObject)),
	};

	buffer_update(ui_elems, sizeof(UIShaderObject), &(UIShaderObject){
		.rect   = rect,
		.colour = colour_normalized(style? style->normal: default_container_style.normal),
		.hl     = RECT0,
		.tex_id = -1,
	}, ui_elemc * sizeof(UIShaderObject));

	ui_elemc++;
	return container;
}

int ui_add(UIContainer* container, UIObject* obj)
{
	/* Check for dead elements to reuse */
	obj->i = free_elems.len > 0? *(int*)varray_pop(&free_elems)
	                           : ui_elemc++;
	varray_push(&container->objects, obj);
	container->has_update = true;

	return obj->i;
}

int ui_add_image(Image* img)
{
	for (int i = 0; i < pipeln_ci.imgc; i++)
		if (pipeln_ci.imgs[i] == img)
			return i;

	pipeln_ci.imgs[pipeln_ci.imgc] = img;
	pipeln_needs_update = true;
	return pipeln_ci.imgc++;
}

void ui_build()
{
	UIContainer* container;
	UIObject*  obj;
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
		container->has_update = false;
	}

	if (pipeln_needs_update) {
		pipeln = pipeln_update(pipeln, &pipeln_ci);
		pipeln_needs_update = false;
	}
}

void ui_update()
{
	context.mouse_pos = VEC2(2.0f*(mouse_x / config.winw - 0.5f), 2.0f*(mouse_y / config.winh - 0.5f));
	Vec2 mouse = context.mouse_pos;

	UIContainer* container;
	UIObject*    obj;
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

	ui_build();
}

void ui_update_object(UIObject* obj)
{
	assert(obj->i >= 0 && obj->i <= ui_elemc);
	if (!obj->style) {
		ERROR("[UI] Object of type %d does not have a style", obj->type);
		obj->style = &default_style;
	}

	Vec4 colour = colour_normalized(obj->state.clicked? obj->style->clicked:
	                                obj->state.hover  ? obj->style->hover  :
	                                                    obj->style->normal);
	buffer_update(ui_elems, sizeof(UIShaderObject), &(UIShaderObject){
		.rect    = obj->rect,
		.uv_rect = obj->uv_rect,
		.colour  = colour,
		.hl      = obj->hl,
		.tex_id  = obj->imgi,
	}, obj->i * sizeof(UIShaderObject));
}

void ui_record_commands(VkCommandBuffer cmd_buf)
{
	Pipeline* pl = pipeln;
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pl->pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pl->layout, 0, 1, &pl->dset.set, 0, NULL);

	int objc = containerc;
	for (int i = 0; i < containerc; i++)
		objc += containers[i].objects.len;
	vkCmdDraw(cmd_buf, 6*objc, 1, 0, 0);
}

void ui_free_container(UIContainer* container)
{
	for (int i = 0; i < containerc; i++) {
		if (container == &containers[i]) {
			container_free(container);
			return;
		}
	}

	ERROR("[UI] Attempt to free invalid container %p", (void*)container);
}

void ui_free()
{
	for (int i = 0; i < containerc; i++)
		container_free(&containers[i]);
	containerc = 0;

	mtx_destroy(&updating_lock);
	sbo_free(&ui_elems);
	pipeln_free(pipeln);
}

/* -------------------------------------------------------------------- */

static void cb_mouse_left(bool kdown)
{
	context.mouse_pressed.lmb  =  kdown;
	context.mouse_released.lmb = !kdown;
}

static void container_free(UIContainer* container)
{
	UIObject* obj;
	while (container->objects.len > 0) {
		obj = varray_pop(&container->objects);
		obj->state.dead = true;
		varray_push(&free_elems, &obj->i);
		switch (obj->type) {
		case UI_LABEL : label_free(obj) ; break;
		case UI_BUTTON: button_free(obj); break;
		case UI_LIST  : uilist_free(obj); break;
		case UI_CUSTOM:
			if (obj->custom.on_free)
				obj->custom.on_free(obj);
			break;
		default:
		}
	}

	varray_free(&container->objects);
	container->state.dead = true;
}

