#include "vulkan/vulkan.h"

#include "config.h"
#include "input.h"
#include "util/varray.h"
#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "types.h"
#include "ui.h"

static struct Pipeline pipeln;
static VkVertexInputBindingDescription vert_binds[] = {
	{ .binding   = 0,
	  .stride    = UI_VERTEX_SIZE, /* xyzrgba */
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
};
static VkVertexInputAttributeDescription vert_attrs[] = {
	{ .binding  = 0,
	  .location = 0,
	  .format   = VK_FORMAT_R32G32B32_SFLOAT, /* xyz */
	  .offset   = 0, },
	{ .binding  = 0,
	  .location = 1,
	  .format   = VK_FORMAT_R32G32B32A32_SFLOAT, /* rgba */
	  .offset   = sizeof(float[3]), }
};

/* -------------------------------------------------------------------- */

struct UIContext ui_context;
struct UIObject  ui_containers[UI_MAX_TOP_LEVEL_CONTAINERS];
int ui_containerc = 0;

static struct VArray ui_objs;

void ui_init(VkRenderPass renderpass)
{
	pipeln = (struct Pipeline){
		// TODO: Tesselation shader?
		.vshader    = create_shader(SHADER_DIR "ui.vert"),
		.fshader    = create_shader(SHADER_DIR "ui.frag"),
		.vert_bindc = 1,
		.vert_binds = vert_binds,
		.vert_attrc = 2,
		.vert_attrs = vert_attrs,
	};
	pipeln_init(&pipeln, renderpass);

	ui_objs = varray_new(UI_DEFAULT_OBJECT_COUNT, sizeof(struct UIObject));

	input_register(SDL_BUTTON_LEFT, LAMBDA(void, bool kdown,
			ui_context.mouse_pressed.lmb  =  kdown;
			ui_context.mouse_released.lmb = !kdown;
		));

	DEBUG(2, "[UI] Initialized UI");
}

struct UIObject* ui_alloc_object() {
	return varray_get(&ui_objs, ui_objs.len++);
}

void ui_build() {
	for (int i = 0; i < ui_containerc; i++)
		container_build(&ui_containers[i]);
}

Rect ui_build_rect(struct UIObject* obj, bool absolute_sz)
{
	float start_x, start_y;
	float rel_w, rel_h;
	struct UIObject* parent = obj->parent;
	if (parent) {
		start_x = parent->rect.x + parent->rect.w/2.0f;
		start_y = parent->rect.y + parent->rect.h/2.0f;
		if (absolute_sz) {
			rel_w = obj->rect.w/global_config.winw*ASPECT_RATIO;
			rel_h = obj->rect.h/global_config.winh*ASPECT_RATIO;
			return RECT(start_x + obj->rect.x*(parent->rect.w/2.0f - rel_w/2.0f) - rel_w/2.0f,
			            start_y + obj->rect.y*(parent->rect.h/2.0f - rel_h/2.0f) - rel_h/2.0f,
			            rel_w, rel_h);
		} else {
			return RECT(start_x + obj->rect.x*(parent->rect.w/2.0f),
			            start_y + obj->rect.y*(parent->rect.h/2.0f),
			            obj->rect.w/parent->rect.w/2.0f,
			            obj->rect.h/parent->rect.h/2.0f);
		}
	}

	return obj->rect;
}

void ui_update()
{
	bool update_ui = false;
	bool prev_state;
	float mx = ((float)mouse_x / global_config.winw - 0.5f)*2.0f;
	float my = ((float)mouse_y / global_config.winh - 0.5f)*2.0f;
	struct UIObject* obj;
	for (int i = 0; i < ui_objs.len; i++) {
		obj = varray_get(&ui_objs, i);
		if (!obj->state.visible || obj->type != UI_BUTTON)
			continue;

		prev_state = obj->state.hover;
		obj->state.hover = (mx >= obj->button.screen_rect.x && mx <= obj->button.screen_rect.x + obj->button.screen_rect.w) &&
		                   (my >= obj->button.screen_rect.y && my <= obj->button.screen_rect.y + obj->button.screen_rect.h);
		if (obj->state.hover != prev_state)
			update_ui = true;

		if (obj->state.hover) {
			if (ui_context.mouse_pressed.lmb && !ui_context.clicked_obj) {
				ui_context.clicked_obj = obj;
			} else if (ui_context.mouse_released.lmb && ui_context.clicked_obj == obj) {
				button_on_click(obj);
				ui_context.clicked_obj = NULL;
			}
		} else {
			if (ui_context.mouse_released.lmb && ui_context.clicked_obj == obj)
				ui_context.clicked_obj = NULL;
		}
	}

	if (update_ui)
		ui_build();
}

void ui_record_commands(VkCommandBuffer cmd_buf)
{
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	for (int i = 0; i < ui_containerc; i++) {
		vkCmdBindVertexBuffers(cmd_buf, 0, 1, &ui_containers[i].container.vbo.buf, (VkDeviceSize[]){ 0 });
		vkCmdDraw(cmd_buf, ui_containers[i].container.verts.len, 1, 0, i);
	}
}

void ui_free()
{
	varray_free(&ui_objs);
	for (int i = 0; i < ui_containerc; i++)
		container_free(&ui_containers[i]);
	pipeln_free(&pipeln);
}
