#include <vulkan/vulkan.h>

#include "gfx/vulkan.h"
#include "gfx/pipeline.h"
#include "scratch.h"

#define SCRATCH_VERTEX_COUNT 7
#define SCRATCH_VERTEX_SIZE  sizeof(float[SCRATCH_VERTEX_COUNT])

static struct Pipeline line_pipeln;
static struct Pipeline plane_pipeln;
static VkVertexInputBindingDescription vert_binds[] = {
	{ .binding   = 0,
	  .stride    = SCRATCH_VERTEX_SIZE, /* xyzrgba */
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, }
};
static VkVertexInputAttributeDescription vert_attrs[] = {
	{ .binding  = 0,
	  .location = 0,
	  .format   = VK_FORMAT_R32G32B32_SFLOAT, /* xyz */
	  .offset   = 0, },
	{ .binding  = 0,
	  .location = 1,
	  .format   = VK_FORMAT_R32G32B32A32_SFLOAT, /* rgba */
	  .offset   = sizeof(float[3]), },
};

void scratch_init(VkRenderPass renderpass)
{
	line_pipeln = (struct Pipeline){
		.vshader     = create_shader(SHADER_DIR "scratch.vert"),
		.fshader     = create_shader(SHADER_DIR "scratch.frag"),
		.vert_bindc  = 1,
		.vert_binds  = vert_binds,
		.vert_attrc  = 2,
		.vert_attrs  = vert_attrs,
		.topology    = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
	};
	plane_pipeln = (struct Pipeline){
		.vshader     = create_shader(SHADER_DIR "scratch.vert"),
		.fshader     = create_shader(SHADER_DIR "scratch.frag"),
		.vert_bindc  = 1,
		.vert_binds  = vert_binds,
		.vert_attrc  = 2,
		.vert_attrs  = vert_attrs,
	};
	pipeln_init(&line_pipeln, renderpass);
	pipeln_init(&plane_pipeln, renderpass);
}

void scratch_record_commands(VkCommandBuffer cmd_buf)
{
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, plane_pipeln.pipeln);
	vkCmdBindVertexBuffers(cmd_buf, 0, 1, NULL, (VkDeviceSize[]) { 0 });
	vkCmdDraw(cmd_buf, -1, 1, 0, 0);

	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, line_pipeln.pipeln);
	vkCmdBindVertexBuffers(cmd_buf, 0, 1, NULL, (VkDeviceSize[]) { 0 });
	vkCmdDraw(cmd_buf, -1, 1, 0, 0);
}

void scratch_free()
{
	pipeln_free(&line_pipeln);
	pipeln_free(&plane_pipeln);
}
