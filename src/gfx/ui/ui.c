#include "vulkan/vulkan.h"

#include "config.h"
#include "util/arena.h"
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
struct Arena*    ui_arena;
struct UIObject* ui_objs[UI_MAX_OBJECTS];
int ui_objc;

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
		.enable_blending = false,
	};
	pipeln_init(&pipeln, renderpass);

	ui_arena = arena_new(UI_ARENA_DEFAULT_SIZE, ARENA_RESIZEABLE);

	DEBUG(2, "[UI] Initialized UI:\n\tArena size: %d", UI_ARENA_DEFAULT_SIZE);
}

void ui_build()
{
	for (int i = 0; i < ui_objc; i++)
		if (ui_objs[i]->type != UI_CONTAINER)
			button_build(ui_objs[i]);
	for (int i = 0; i < ui_objc; i++)
		if (ui_objs[i]->type == UI_CONTAINER)
			container_build(ui_objs[i]);
}

Rect ui_build_rect(struct UIObject* obj, bool absolute_sz)
{
	float margin_x = (float)obj->style->margin / global_config.winw;
	float margin_y = (float)obj->style->margin / global_config.winh;

	float start_x, start_y;
	float scale_x, scale_y;
	if (obj->parent != -1) {
		struct Rect parent_rect = ui_objs[obj->parent]->rect;
		float parent_margin = ui_objs[obj->parent]->style->margin;
		if (absolute_sz) {
			start_x = parent_rect.x + (parent_margin + margin_x)/global_config.winw;
			start_y = parent_rect.y + (parent_margin + margin_y)/global_config.winh;
		} else {
			start_x = parent_rect.x + parent_margin + margin_x;
			start_y = parent_rect.y + parent_margin + margin_y;
		}
		scale_x = parent_rect.w - 2.0f*margin_x;
		scale_y = parent_rect.h - 2.0f*margin_y;
	} else {
		start_x = 0.0f;
		start_y = 0.0f;
		scale_x = 1.0f;
		scale_y = 1.0f;
	}

	return RECT(start_x + obj->rect.x*scale_x + margin_x,
	            start_y + obj->rect.y*scale_y + margin_y,
	            absolute_sz? obj->rect.w/global_config.winw: obj->rect.w*scale_x - 2.0f*margin_x,
	            absolute_sz? obj->rect.h/global_config.winh: obj->rect.h*scale_y - 2.0f*margin_y);
}

void ui_record_commands(VkCommandBuffer cmd_buf)
{
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, &pipeln.dset, 0, NULL);
	struct Container* container;
	for (int i = 0; i < ui_objc; i++) {
		if (ui_objs[i]->type != UI_CONTAINER)
			continue;
		container = ui_objs[i]->data;
		vkCmdBindVertexBuffers(cmd_buf, 0, 1, &container->vbo.buf, (VkDeviceSize[]){ 0 });
		vkCmdDraw(cmd_buf, container->vertc, 1, 0, i);
	}
}

void ui_free()
{
	for (int i = 0; i < ui_objc; i++)
		if (ui_objs[i]->type == UI_CONTAINER)
			vbo_free(&((struct Container*)ui_objs[i]->data)->vbo);
	pipeln_free(&pipeln);
}
