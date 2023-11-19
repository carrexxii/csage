#include "vulkan/vulkan.h"

#include "config.h"
#include "util/arena.h"
#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "ui.h"
#include <vulkan/vulkan_core.h>

#define SIZEOF_UI_VERTEX sizeof(float[7])

static struct Pipeline pipeln;
static VkVertexInputBindingDescription vert_binds[] = {
	{ .binding   = 0,
	  .stride    = SIZEOF_UI_VERTEX, /* xyzrgba */
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
		.topology        = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
	};
	pipeln_init(&pipeln, renderpass);

	ui_arena = arena_new(UI_ARENA_DEFAULT_SIZE, ARENA_RESIZEABLE);

	DEBUG(2, "[UI] Initialized UI:\n\tArena size: %d", UI_ARENA_DEFAULT_SIZE);
}

void ui_build()
{
	for (int i = 0; i < ui_objc; i++)
		container_build(ui_objs[i]);
}

void ui_record_commands(VkCommandBuffer cmd_buf)
{
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, &pipeln.dset, 0, NULL);
	struct Container* container;
	for (int i = 0; i < ui_objc; i++) {
		container = ui_objs[i]->data;
		vkCmdBindVertexBuffers(cmd_buf, 0, 1, &container->vbo.buf, (VkDeviceSize[]){ 0 });
		vkCmdDraw(cmd_buf, container->vertc, 1, 0, i);
	}
}

void ui_free()
{
	for (int i = 0; i < ui_objc; i++)
		vbo_free(&((struct Container*)ui_objs[i]->data)->vbo);
	pipeln_free(&pipeln);
}
