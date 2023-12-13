#include <vulkan/vulkan.h>

#include "util/varray.h"
#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "gfx/pipeline.h"
#include "gfx/primitives.h"
#include "input.h"
#include "camera.h"
#include "maths.h"
#include "pga.h"
#include "scratch.h"

#define SCRATCH_VERTEX_COUNT 7
#define SCRATCH_VERTEX_SIZE  sizeof(float[SCRATCH_VERTEX_COUNT])

static void scratch_add_point(float a[7]);
static void scratch_add_line(float a[7], float b[7]);

static struct Pipeline point_pipeln;
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
	{ -100.0f, 0.0f, 0.0f, COLOUR_RED, 1.0f },
	{  100.0f, 0.0f, 0.0f, COLOUR_RED, 1.0f },
	/* y */
	{ 0.0f, -100.0f, 0.0f, COLOUR_GREEN, 1.0f },
	{ 0.0f,  100.0f, 0.0f, COLOUR_GREEN, 1.0f },
	/* z */
	{ 0.0f, 0.0f, -100.0f, COLOUR_BLUE, 1.0f },
	{ 0.0f, 0.0f,  100.0f, COLOUR_BLUE, 1.0f },
};

static UBO cam_ubo;
static VBO axes_vbo;
static VBO points_vbo;
static VBO lines_vbo;
static VBO planes_vbo;
static struct VArray points;
static struct VArray lines;
static struct VArray planes;

void scratch_init(VkRenderPass renderpass)
{
	cam_ubo  = ubo_new(sizeof(Mat4x4[2]));
	axes_vbo = vbo_new(sizeof(axes_verts), axes_verts, false);

	point_pipeln = (struct Pipeline){
		.vshader     = create_shader(SHADER_DIR "scratch.vert"),
		.fshader     = create_shader(SHADER_DIR "scratch.frag"),
		.vert_bindc  = 1,
		.vert_binds  = vert_binds,
		.vert_attrc  = 2,
		.vert_attrs  = vert_attrs,
		.topology    = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
		.uboc        = 1,
		.dset_cap    = 1,
	};
	pipeln_alloc_dsets(&point_pipeln);
	pipeln_create_dset(&point_pipeln, 1, &cam_ubo, 0, NULL, 0, NULL);
	pipeln_init(&point_pipeln, renderpass);

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

	points = varray_new(SCRATCH_DEFAULT_ELEMENT_COUNT, sizeof(float[1][7]));
	lines  = varray_new(SCRATCH_DEFAULT_ELEMENT_COUNT, sizeof(float[2][7]));
	planes = varray_new(SCRATCH_DEFAULT_ELEMENT_COUNT, sizeof(float[6][7]));
}

void scratch_load()
{

}

void scratch_add_bivec(Bivec a)
{
	float r = a.d.x*a.d.x + a.d.y*a.d.y + a.d.z*a.d.z;
	Vec3 p = dual(wedge(VEC(a.d.x, a.d.y, a.d.z, 0.0), VEC(a.m.x, a.m.y, a.m.z, 1.0))).m;
	// product(p, norm2(a.d));
	p.x /= r;
	p.y /= r;
	p.z /= r;
	Vec3 s = { p.x - 100.0f*a.d.x, p.y - 100.0f*a.d.y, p.z - 100.0f*a.d.z };
	Vec3 t = { p.x + 100.0f*a.d.x, p.y + 100.0f*a.d.y, p.z + 100.0f*a.d.z };
	scratch_add_line((float[7]){ s.x, s.y, s.z, COLOUR_BLUE, 1.0f },
	                 (float[7]){ t.x, t.y, t.z, COLOUR_BLUE, 1.0f });
}

void scratch_add_trivec(Trivec a)
{
	float d = 1.0f / a.w;
	scratch_add_point((float[7]){ a.x*d, a.y*d, a.z*d, COLOUR_WHITE, 1.0f, });
}

void scratch_clear()
{
	varray_reset(&points);
	varray_reset(&lines);
	varray_reset(&planes);
	vbo_free(&points_vbo);
	vbo_free(&lines_vbo);
	vbo_free(&planes_vbo);
}

void scratch_record_commands(VkCommandBuffer cmd_buf, struct Camera* cam)
{
	buffer_update(cam_ubo, cam_ubo.sz, cam->mats, 0);

	if (points.len > 0) {
		if (!points_vbo.sz)
			points_vbo = vbo_new(SCRATCH_VERTEX_SIZE*points.len, points.data, false);
		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, point_pipeln.pipeln);
		vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, point_pipeln.layout, 0, 1, point_pipeln.dsets, 0, NULL);
		vkCmdBindVertexBuffers(cmd_buf, 0, 1, &points_vbo.buf, (VkDeviceSize[]) { 0 });
		vkCmdDraw(cmd_buf, points.len, 1, 0, 0);
	}

	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, line_pipeln.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, line_pipeln.layout, 0, 1, line_pipeln.dsets, 0, NULL);
	vkCmdBindVertexBuffers(cmd_buf, 0, 1, &axes_vbo.buf, (VkDeviceSize[]) { 0 });
	vkCmdDraw(cmd_buf, 6, 1, 0, 0);
	if (lines.len > 0) {
		if (!lines_vbo.sz)
			lines_vbo = vbo_new(lines.len*lines.elem_sz, lines.data, false);
		vkCmdBindVertexBuffers(cmd_buf, 0, 1, &lines_vbo.buf, (VkDeviceSize[]) { 0 });
		vkCmdDraw(cmd_buf, 2*lines.len, 1, 0, 0);
	}

	if (planes.len > 0) {
		if (!planes_vbo.sz)
			planes_vbo = vbo_new(SCRATCH_VERTEX_SIZE*planes.len, planes.data, false);
		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, plane_pipeln.pipeln);
		vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, plane_pipeln.layout, 0, 1, plane_pipeln.dsets, 0, NULL);
		vkCmdBindVertexBuffers(cmd_buf, 0, 1, &planes_vbo.buf, (VkDeviceSize[]) { 0 });
		vkCmdDraw(cmd_buf, 6, 1, 0, 0);
	}
}

void scratch_free()
{
	ubo_free(&cam_ubo);
	vbo_free(&axes_vbo);
	if (points_vbo.sz) vbo_free(&points_vbo);
	if (lines_vbo.sz)  vbo_free(&lines_vbo);
	if (planes_vbo.sz) vbo_free(&planes_vbo);
	pipeln_free(&line_pipeln);
	pipeln_free(&plane_pipeln);

	varray_free(&points);
	varray_free(&lines);
	varray_free(&planes);
}

/* -------------------------------------------------------------------- */

static void scratch_add_point(float a[7])
{
	varray_push(&points, (float[7]){ UNPACK7(a) });
}

static void scratch_add_line(float a[7], float b[7])
{
	varray_push(&lines, (float[2][7]){ UNPACK7(a), UNPACK7(b) });
}
