#include <vulkan/vulkan.h>

#include "util/varray.h"
#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "gfx/pipeline.h"
#include "gfx/primitives.h"
#include "input.h"
#include "camera.h"
#include "maths.h"
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

static float axes_verts[][7] = {
	/* x */
	{ -10.0f, 0.0f, 0.0f, COLOUR_RED, 1.0f },
	{  10.0f, 0.0f, 0.0f, COLOUR_RED, 1.0f },
	/* y */
	{ 0.0f, -10.0f, 0.0f, COLOUR_GREEN, 1.0f },
	{ 0.0f,  10.0f, 0.0f, COLOUR_GREEN, 1.0f },
	/* z */
	{ 0.0f, 0.0f, -10.0f, COLOUR_BLUE, 1.0f },
	{ 0.0f, 0.0f,  10.0f, COLOUR_BLUE, 1.0f },
};

static UBO cam_ubo;
static VBO axes_vbo;
static VBO lines_vbo;
static VBO planes_vbo;
static struct VArray lines;
static struct VArray planes;

void scratch_init(VkRenderPass renderpass)
{
	cam_ubo = ubo_new(sizeof(Mat4x4[2]));

	axes_vbo = vbo_new(sizeof(axes_verts), axes_verts, false);
	line_pipeln = (struct Pipeline){
		.vshader     = create_shader(SHADER_DIR "scratch.vert"),
		.fshader     = create_shader(SHADER_DIR "scratch.frag"),
		.vert_bindc  = 1,
		.vert_binds  = vert_binds,
		.vert_attrc  = 2,
		.vert_attrs  = vert_attrs,
		.topology    = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
		.uboc        = 1,
		.dset_cap    = 1,
	};
	pipeln_alloc_dsets(&line_pipeln);
	pipeln_create_dset(&line_pipeln, 1, &cam_ubo, 0, NULL, 0, NULL);
	pipeln_init(&line_pipeln, renderpass);

	plane_pipeln = (struct Pipeline){
		.vshader     = create_shader(SHADER_DIR "scratch.vert"),
		.fshader     = create_shader(SHADER_DIR "scratch.frag"),
		.vert_bindc  = 1,
		.vert_binds  = vert_binds,
		.vert_attrc  = 2,
		.vert_attrs  = vert_attrs,
		.uboc        = 1,
		.dset_cap    = 1,
	};
	pipeln_alloc_dsets(&plane_pipeln);
	pipeln_create_dset(&plane_pipeln, 1, &cam_ubo, 0, NULL, 0, NULL);
	pipeln_init(&plane_pipeln, renderpass);

	lines  = varray_new(SCRATCH_DEFAULT_ELEMENT_COUNT, sizeof(float[2][7]));
	planes = varray_new(SCRATCH_DEFAULT_ELEMENT_COUNT, sizeof(float[6][7]));

	float p[6*7];
	quad_from_rect(p, RECT(0.0, 0.0, 2.0, 1.0), 0.0, COLOUR(0xFF0000FF));
	planes_vbo = vbo_new(sizeof(p), p, false);
	varray_push(&planes, p);
}

void scratch_load()
{

}

void scratch_clear()
{
	varray_reset(&lines);
	varray_reset(&planes);
}

void scratch_record_commands(VkCommandBuffer cmd_buf, struct Camera* cam)
{
	buffer_update(cam_ubo, cam_ubo.sz, cam->mats, 0);

	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, line_pipeln.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, line_pipeln.layout, 0, 1, line_pipeln.dsets, 0, NULL);
	vkCmdBindVertexBuffers(cmd_buf, 0, 1, &axes_vbo.buf, (VkDeviceSize[]) { 0 });
	vkCmdDraw(cmd_buf, 6, 1, 0, 0);
	if (lines.len > 0) {
		vkCmdBindVertexBuffers(cmd_buf, 0, 1, &lines_vbo.buf, (VkDeviceSize[]) { 0 });
		vkCmdDraw(cmd_buf, lines.len, 1, 0, 0);
	}

	if (planes.len > 0) {
		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, plane_pipeln.pipeln);
		vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, plane_pipeln.layout, 0, 1, plane_pipeln.dsets, 0, NULL);
		vkCmdBindVertexBuffers(cmd_buf, 0, 1, &planes_vbo.buf, (VkDeviceSize[]) { 0 });
		vkCmdDraw(cmd_buf, 6, 1, 0, 0);
	}
}

void scratch_free()
{
	ubo_free(&cam_ubo);
	pipeln_free(&line_pipeln);
	pipeln_free(&plane_pipeln);
}
