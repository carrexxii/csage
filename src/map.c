#include "vulkan/vulkan.h"

#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "gfx/pipeline.h"
#include "camera.h"
#include "map.h"

#define BLOCK_WIDTH          32
#define BLOCK_HEIGHT         BLOCK_WIDTH
#define BLOCK_DEPTH          16
#define BLOCKS_PER_LAYER     (BLOCK_WIDTH*BLOCK_HEIGHT)
#define VERTEX_WIDTH         (BLOCK_WIDTH + 1)
#define VERTEX_HEIGHT        (BLOCK_HEIGHT + 1)
#define VERTEX_DEPTH         (BLOCK_DEPTH + 1)
#define VERTICES_PER_LAYER   (VERTEX_WIDTH*VERTEX_HEIGHT)
#define VERTICES_PER_BLOCK   VERTEX_WIDTH*VERTEX_HEIGHT*VERTEX_DEPTH
#define VOXELS_PER_LAYER     BLOCK_WIDTH*BLOCK_HEIGHT
#define VOXELS_PER_BLOCK     VOXELS_PER_LAYER*BLOCK_DEPTH
#define TRIANGLES_PER_VOXEL  6
#define VERTICES_PER_VOXEL   3*TRIANGLES_PER_VOXEL
#define TRIANGLES_PER_BLOCK  TRIANGLES_PER_VOXEL*VOXELS_PER_BLOCK
#define VERTEX_ELEMENT_COUNT 6
#define SIZEOF_VERTEX        sizeof(int8[VERTEX_ELEMENT_COUNT])

static void remesh_block(int b);

/* -------------------------------------------------------------------- */
static VkVertexInputBindingDescription vert_binds[] = {
	/* xyznnnuv */
	{ .binding   = 0,
	  .stride    = SIZEOF_VERTEX,
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
};
static VkVertexInputAttributeDescription vertex_attrs[] = {
	/* xyz */
	{ .binding  = 0,
	  .location = 0,
	  .format   = VK_FORMAT_R8G8B8_SINT,
	  .offset   = 0, },
	/* nnn */
	{ .binding  = 0,
	  .location = 1,
	  .format   = VK_FORMAT_R8G8B8_SINT,
	  .offset   = sizeof(int8[3]), },
};
/* -------------------------------------------------------------------- */

static struct Pipeline pipeln;
static UBO  ubo_buf;
static VBO  vbo_buf;
static IBO* ibo_bufs;
static ivec3s size;
static struct Voxel* blocks;
static int blockc;

void map_init(VkRenderPass render_pass)
{
	ubo_buf = ubo_new(sizeof(mat4));
	pipeln = (struct Pipeline){
		.vshader    = create_shader(SHADER_DIR "map.vert"),
		.fshader    = create_shader(SHADER_DIR "map.frag"),
		.vertbindc  = 1,
		.vertbinds  = vert_binds,
		.vertattrc  = ARRAY_LEN(vertex_attrs),
		.vertattrs  = vertex_attrs,
		.uboc       = 1,
		.ubos       = &ubo_buf,
	};
	pipeln_init(&pipeln, render_pass);
	DEBUG(4, "[MAP] Map initialized. Block dimensions are set to: %dx%dx%d",
	      BLOCK_WIDTH, BLOCK_HEIGHT, BLOCK_DEPTH);
}

void map_new(ivec3s dim)
{
	int fe_rounding = fegetround();
	fesetround(FE_UPWARD);
	size = (ivec3s){
		rintf((float)dim.x/(float)BLOCK_WIDTH),
		rintf((float)dim.y/(float)BLOCK_HEIGHT),
		rintf((float)dim.z/(float)BLOCK_DEPTH),
	};
	blockc = size.x*size.y*size.z;
	fesetround(fe_rounding);

	blocks = scalloc(blockc, VOXELS_PER_BLOCK*sizeof(struct Voxel));

	/* Generate the vertex lattice -> 3 versions, 1 for each normal */
	intptr vert_size = 3*VERTEX_DEPTH*VERTEX_HEIGHT*VERTEX_WIDTH*VERTEX_ELEMENT_COUNT*sizeof(int8);
	int8* verts = scalloc(vert_size, 1);
	int i;
	for (int w = 0; w < 3; w++) {
		for (int z = 0; z < VERTEX_DEPTH; z++) {
			for (int y = 0; y < VERTEX_HEIGHT; y++) {
				for (int x = 0; x < VERTEX_WIDTH; x++) {
					i = VERTEX_ELEMENT_COUNT*(z*VERTICES_PER_LAYER + y*VERTEX_WIDTH + x) + VERTEX_ELEMENT_COUNT*w*VERTICES_PER_BLOCK;
					verts[i + 0] = x;
					verts[i + 1] = y;
					verts[i + 2] = z;
					verts[i + 3 + w] = 1;
					// DEBUG(1, "[%d] %hhd %hhd %hhd (%hhd %hhd %hhd)", i, verts[i], verts[i+1], verts[i+2], verts[i+3], verts[i+4], verts[i+5]);
				}
			}
		}
	}
	vbo_buf = vbo_new(vert_size, verts);
	free(verts);

	ibo_bufs = scalloc(blockc, sizeof(IBO));
	for (int b = 0; b < blockc; b++)
		remesh_block(b);

	DEBUG(3, "[MAP] Initialized map with dimensions: %dx%dx%d (%d blocks)",
	      size.x*BLOCK_WIDTH, size.y*BLOCK_HEIGHT, size.z*BLOCK_DEPTH, blockc);
}

void map_record_commands(VkCommandBuffer cmd_buf)
{
	mat4 vp;
	camera_get_vp(vp);
	buffer_update(ubo_buf, sizeof(mat4), vp);

	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, &pipeln.dset, 0, NULL);

	vkCmdBindVertexBuffers(cmd_buf, 0, 1, &vbo_buf.buf, (VkDeviceSize[]){ 0 });
	for (int i = 0; i < blockc; i++) {
		// DEBUG(1, "[%d] Drawing %d vertices", i, models[i].meshes[m].vertc);
		vkCmdBindIndexBuffer(cmd_buf, ibo_bufs[i].buf, 0, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(cmd_buf, 3*TRIANGLES_PER_BLOCK, 1, 0, 0, 0);
	}
}

void map_free()
{
	vbo_free(&vbo_buf);
	for (int i = 0; i < blockc; i++)
		ibo_free(&ibo_bufs[i]);

	free(ibo_bufs);
	free(blocks);

	pipeln_free(&pipeln);
}

/* 0 = x-axis; 1 = y-axis; 2 = z-axis */
#define VERTEX_INDEX(x, y, z, axis) ((z)*VERTICES_PER_LAYER + (y)*VERTEX_WIDTH + (x) + axis*VERTICES_PER_BLOCK)
static void remesh_block(int b)
{
	intptr inds_size = 3*TRIANGLES_PER_BLOCK*sizeof(uint16);
	int16* inds_data = smalloc(inds_size);
	int16* inds = inds_data;
	for (int z = 0; z < BLOCK_DEPTH; z++) {
		for (int y = 0; y < BLOCK_HEIGHT; y++) {
			for (int x = 0; x < BLOCK_WIDTH; x++) {
				/* Left side triangles */
				*inds++ = VERTEX_INDEX(x    , y + 1, z    , 1);
				*inds++ = VERTEX_INDEX(x    , y + 1, z + 1, 1);
				*inds++ = VERTEX_INDEX(x + 1, y + 1, z    , 1);
				*inds++ = VERTEX_INDEX(x + 1, y + 1, z    , 1);
				*inds++ = VERTEX_INDEX(x    , y + 1, z + 1, 1);
				*inds++ = VERTEX_INDEX(x + 1, y + 1, z + 1, 1);

				/* Top triangles */
				*inds++ = VERTEX_INDEX(x    , y    , z, 2);
				*inds++ = VERTEX_INDEX(x    , y + 1, z, 2);
				*inds++ = VERTEX_INDEX(x + 1, y    , z, 2);
				*inds++ = VERTEX_INDEX(x + 1, y    , z, 2);
				*inds++ = VERTEX_INDEX(x    , y + 1, z, 2);
				*inds++ = VERTEX_INDEX(x + 1, y + 1, z, 2);

				/* Right side triangles */
				*inds++ = VERTEX_INDEX(x + 1, y + 1, z    , 0);
				*inds++ = VERTEX_INDEX(x + 1, y + 1, z + 1, 0);
				*inds++ = VERTEX_INDEX(x + 1, y    , z    , 0);
				*inds++ = VERTEX_INDEX(x + 1, y    , z    , 0);
				*inds++ = VERTEX_INDEX(x + 1, y + 1, z + 1, 0);
				*inds++ = VERTEX_INDEX(x + 1, y    , z + 1, 0);
			}
		}
	}
	ibo_bufs[b] = ibo_new(inds_size, inds_data);
	free(inds_data);
}
