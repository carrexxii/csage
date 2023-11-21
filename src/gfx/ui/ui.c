#include "vulkan/vulkan.h"

#include "config.h"
#include "input.h"
#include "util/varray.h"
#include "gfx/vulkan.h"
#include "gfx/buffers.h"
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
struct VArray*   ui_objs;
struct UIObject  ui_containers[UI_MAX_TOP_LEVEL_CONTAINERS];
int ui_containerc = 0;

static enum MouseMask mouse_state;

void ui_init(VkRenderPass renderpass)
{
	pipeln = (struct Pipeline){
		.vshader         = create_shader(SHADER_DIR "ui.vert"),
		.fshader         = create_shader(SHADER_DIR "ui.frag"),
		// TODO: Tesselation shader?
		.vert_bindc      = 1,
		.vert_binds      = vert_binds,
		.vert_attrc      = 2,
		.vert_attrs      = vert_attrs,
	};
	pipeln_init(&pipeln, renderpass);

	ui_objs = varray_new(UI_DEFAULT_OBJECT_COUNT, sizeof(struct UIObject));

	input_register(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, LAMBDA(void, void, mouse_state |=  MOUSE_MASK_LEFT;));
	input_register(SDL_MOUSEBUTTONUP  , SDL_BUTTON_LEFT, LAMBDA(void, void, mouse_state &= ~MOUSE_MASK_LEFT;));

	DEBUG(2, "[UI] Initialized UI");
}

struct UIObject* ui_alloc_object() {
	return varray_get(ui_objs, ui_objs->len++);
}
String ui_alloc_string(char* text, isize len) {
	return string_new(text, len);
}

void ui_build() {
	for (int i = 0; i < ui_containerc; i++)
		container_build(&ui_containers[i]);
}

Rect ui_build_rect(struct UIObject* obj, bool absolute_sz)
{
	float start_x, start_y;
	float scale_x, scale_y;
	struct UIObject* parent = obj->parent;
	if (parent) {
		if (absolute_sz) {
			start_x = parent->rect.x + parent->rect.w/2.0f - obj->rect.w/global_config.winw;
			start_y = parent->rect.y - parent->rect.h/2.0f - obj->rect.h/global_config.winh;
			scale_x = parent->rect.w/2.0f - obj->rect.w/global_config.winw;
			scale_y = parent->rect.h/2.0f - obj->rect.h/global_config.winh;
		} else {
			// TODO + the return
			start_x = 0.0f;
			start_y = -1.0f;
			scale_x = 1.0f;
			scale_y = 1.0f;
		}
	} else {
		start_x = 0.0f;
		start_y = -1.0f;
		scale_x = 1.0f;
		scale_y = 1.0f;
	}

	return RECT(start_x + obj->rect.x*scale_x,
	            start_y + obj->rect.y*scale_y,
	            absolute_sz? 2.0f*obj->rect.w/global_config.winw: obj->rect.w*scale_x,
	            absolute_sz? 2.0f*obj->rect.h/global_config.winh: obj->rect.h*scale_y);
}

void ui_update()
{
	bool update_ui = false;
	bool prev_state;
	float mx = (float)mouse_x / global_config.winw;
	float my = (float)mouse_y / global_config.winh;
	struct UIObject* obj;
	struct UIObject* cont;
	for (int i = 0; i < ui_containerc; i++) {
		cont = &ui_containers[i];
		for (int j = 0; j < cont->container.objs->len; j++) {
			obj = varray_get(cont->container.objs, j);
			prev_state = obj->state.hover;
			obj->state.hover = (mx >= obj->screen_rect.x && mx <= obj->screen_rect.x + obj->screen_rect.w) &&
			                   (my >= obj->screen_rect.y && my <= obj->screen_rect.y + obj->screen_rect.h);
			if (obj->state.hover != prev_state) {
				button_on_hover(obj);
				update_ui = true;
			}
		}
	}

	if (update_ui)
		ui_build();
}

void ui_record_commands(VkCommandBuffer cmd_buf)
{
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, &pipeln.dset, 0, NULL);
	for (int i = 0; i < ui_containerc; i++) {
		vkCmdBindVertexBuffers(cmd_buf, 0, 1, &ui_containers[i].container.vbo.buf, (VkDeviceSize[]){ 0 });
		vkCmdDraw(cmd_buf, ui_containers[i].container.verts->len, 1, 0, i);
	}
}

void ui_free()
{
	varray_free(ui_objs);
	for (int i = 0; i < ui_containerc; i++)
		container_free(&ui_containers[i]);
	pipeln_free(&pipeln);
}
