#include <vulkan/vulkan.h>

#include "resmgr.h"
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
static void scratch_add_plane(float pts[6][7]);

static struct Pipeline* point_pipeln;
static struct Pipeline* line_pipeln;
static struct Pipeline* plane_pipeln;
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
	{ -100.0f, 0.0f, 0.0f, COLOUR_RED, SCRATCH_AXIS_OPACITY },
	{  100.0f, 0.0f, 0.0f, COLOUR_RED, SCRATCH_AXIS_OPACITY },
	/* y */
	{ 0.0f, -100.0f, 0.0f, COLOUR_GREEN, SCRATCH_AXIS_OPACITY },
	{ 0.0f,  100.0f, 0.0f, COLOUR_GREEN, SCRATCH_AXIS_OPACITY },
	/* z */
	{ 0.0f, 0.0f, -100.0f, COLOUR_BLUE, SCRATCH_AXIS_OPACITY },
	{ 0.0f, 0.0f,  100.0f, COLOUR_BLUE, SCRATCH_AXIS_OPACITY },
};

static UBO cam_ubo;
static VBO axes_vbo;
static VBO points_vbo;
static VBO lines_vbo;
static VBO planes_vbo;
static struct VArray points;
static struct VArray lines;
static struct VArray planes;
static bool axes_on;
static bool points_on;
static bool lines_on;
static bool planes_on;

void scratch_init()
{
	cam_ubo  = ubo_new(sizeof(Mat4x4[2]));
	axes_vbo = vbo_new(sizeof(axes_verts), axes_verts, false);

	struct PipelineCreateInfo pipeln_ci = {
		.vshader     = STRING(SHADER_PATH "/scratch.vert"),
		.fshader     = STRING(SHADER_PATH "/scratch.frag"),
		.topology    = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
		.vert_bindc  = ARRAY_SIZE(vert_binds),
		.vert_binds  = vert_binds,
		.vert_attrc  = ARRAY_SIZE(vert_attrs),
		.vert_attrs  = vert_attrs,
		.uboc        = 1,
		.ubos[0]     = cam_ubo,
	};
	point_pipeln = pipeln_new(&pipeln_ci, "Scratch points");
	point_pipeln = pipeln_update(point_pipeln, &pipeln_ci);

	pipeln_ci.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	line_pipeln = pipeln_new(&pipeln_ci, "Scratch lines");
	line_pipeln = pipeln_update(line_pipeln, &pipeln_ci);

	pipeln_ci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	plane_pipeln = pipeln_new(&pipeln_ci, "Scratch planes");
	plane_pipeln = pipeln_update(plane_pipeln, &pipeln_ci);

	points = varray_new(SCRATCH_DEFAULT_ELEMENT_COUNT, sizeof(float[1][7]));
	lines  = varray_new(SCRATCH_DEFAULT_ELEMENT_COUNT, sizeof(float[2][7]));
	planes = varray_new(SCRATCH_DEFAULT_ELEMENT_COUNT, sizeof(float[6][7]));

	axes_on   = true;
	points_on = true;
	lines_on  = true;
	planes_on = true;
}

static void cb_toggle_axes(bool kdown)   { if (kdown) axes_on   = !axes_on;   }
static void cb_toggle_points(bool kdown) { if (kdown) points_on = !points_on; }
static void cb_toggle_lines(bool kdown)  { if (kdown) lines_on  = !lines_on;  }
static void cb_toggle_planes(bool kdown) { if (kdown) planes_on = !lines_on;  }
void scratch_load()
{
	input_register(SDLK_1, cb_toggle_axes);
	input_register(SDLK_2, cb_toggle_lines);
	input_register(SDLK_3, cb_toggle_planes);
	input_register(SDLK_4, cb_toggle_points);
}

void scratch_add_vec(Vec a)
{
	/* A(a + 1) + Bb + C + D = 0 */
	float s = norm(a)/2.0f;
	float d = 1.0f/(norm(a)/a.w);
	Vec3  n = multiply(normalized(a.v), -1.0f*d);
	float p1[7] = { n.x + s, n.y    , n.z - s*(n.x/n.z), SCRATCH_PLANE_COLOUR, SCRATCH_OPACITY, };
	float p2[7] = { n.x    , n.y + s, n.z - s*(n.y/n.z), SCRATCH_PLANE_COLOUR, SCRATCH_OPACITY, };
	float p3[7] = { n.x - s, n.y    , n.z + s*(n.x/n.z), SCRATCH_PLANE_COLOUR, SCRATCH_OPACITY, };
	float p4[7] = { n.x    , n.y - s, n.z + s*(n.y/n.z), SCRATCH_PLANE_COLOUR, SCRATCH_OPACITY, };
	scratch_add_plane((float[6][7]){ UNPACK7(p1), UNPACK7(p2), UNPACK7(p3),
	                                 UNPACK7(p3), UNPACK7(p4), UNPACK7(p1), });
}

void scratch_add_bivec(Bivec a)
{
	/* p = !(a.d ∧ a.m) / ||a.d||^2 */
	Vec3 p = multiply(dual(wedge(DVEC(a.d), PVEC(a.m))).m, 1.0f/norm2(a.d));
	Vec3 s = { p.x - 100.0f*a.d.x, p.y - 100.0f*a.d.y, p.z - 100.0f*a.d.z };
	Vec3 t = { p.x + 100.0f*a.d.x, p.y + 100.0f*a.d.y, p.z + 100.0f*a.d.z };
	scratch_add_line((float[7]){ s.x, s.y, s.z, SCRATCH_LINE_COLOUR, 1.0f },
	                 (float[7]){ t.x, t.y, t.z, SCRATCH_LINE_COLOUR, 1.0f });
}

void scratch_add_trivec(Trivec a)
{
	Trivec p = a;
	scratch_add_point((float[7]){ p.x, p.y, p.z, SCRATCH_POINT_COLOUR, 1.0f, });
}

void scratch_add_trivecs(Trivec as[4])
{
	scratch_add_plane((float[6][7]){
		{ UNPACK3(as[0].arr), SCRATCH_POINT_COLOUR, SCRATCH_OPACITY },
		{ UNPACK3(as[1].arr), SCRATCH_POINT_COLOUR, SCRATCH_OPACITY },
		{ UNPACK3(as[2].arr), SCRATCH_POINT_COLOUR, SCRATCH_OPACITY },

		{ UNPACK3(as[2].arr), SCRATCH_POINT_COLOUR, SCRATCH_OPACITY },
		{ UNPACK3(as[3].arr), SCRATCH_POINT_COLOUR, SCRATCH_OPACITY },
		{ UNPACK3(as[0].arr), SCRATCH_POINT_COLOUR, SCRATCH_OPACITY },
	});
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

void scratch_record_commands(VkCommandBuffer cmd_buf)
{
	if (points_on && points.len > 0) {
		if (!points_vbo.sz)
			points_vbo = vbo_new(points.len*points.elem_sz, points.data, false);
		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, point_pipeln->pipeln);
		vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, point_pipeln->layout, 0, 1, &point_pipeln->dset.set, 0, NULL);
		vkCmdBindVertexBuffers(cmd_buf, 0, 1, &points_vbo.buf, (VkDeviceSize[]){ 0 });
		vkCmdDraw(cmd_buf, points.len, 1, 0, 0);
	}

	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, line_pipeln->pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, line_pipeln->layout, 0, 1, &line_pipeln->dset.set, 0, NULL);
	if (axes_on) {
		vkCmdBindVertexBuffers(cmd_buf, 0, 1, &axes_vbo.buf, (VkDeviceSize[]){ 0 });
		vkCmdDraw(cmd_buf, 6, 1, 0, 0);
	}
	if (lines_on && lines.len > 0) {
		if (!lines_vbo.sz)
			lines_vbo = vbo_new(lines.len*lines.elem_sz, lines.data, false);
		vkCmdBindVertexBuffers(cmd_buf, 0, 1, &lines_vbo.buf, (VkDeviceSize[]){ 0 });
		vkCmdDraw(cmd_buf, 2*lines.len, 1, 0, 0);
	}

	if (planes_on && planes.len > 0) {
		if (!planes_vbo.sz)
			planes_vbo = vbo_new(planes.len*planes.elem_sz, planes.data, false);
		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, plane_pipeln->pipeln);
		vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, plane_pipeln->layout, 0, 1, &plane_pipeln->dset.set, 0, NULL);
		vkCmdBindVertexBuffers(cmd_buf, 0, 1, &planes_vbo.buf, (VkDeviceSize[]){ 0 });
		vkCmdDraw(cmd_buf, 6*planes.len, 1, 0, 0);
	}
}

void scratch_free()
{
	ubo_free(&cam_ubo);
	vbo_free(&axes_vbo);
	if (points_vbo.sz) vbo_free(&points_vbo);
	if (lines_vbo.sz)  vbo_free(&lines_vbo);
	if (planes_vbo.sz) vbo_free(&planes_vbo);
	pipeln_free(point_pipeln);
	pipeln_free(line_pipeln);
	pipeln_free(plane_pipeln);

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

static void scratch_add_plane(float pts[6][7])
{
	varray_push(&planes, (float[6][7]){ UNPACK7(pts[0]), UNPACK7(pts[1]), UNPACK7(pts[2]),
	                                    UNPACK7(pts[3]), UNPACK7(pts[4]), UNPACK7(pts[5]), });
}
