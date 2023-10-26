#include "common.h"
#include "vulkan/vulkan.h"

#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "gfx/pipeline.h"
#include "camera.h"
#include "map.h"

#define TRIANGLES_PER_VOXEL  6
#define VERTICES_PER_VOXEL   3*TRIANGLES_PER_VOXEL
#define TRIANGLES_PER_BLOCK  TRIANGLES_PER_VOXEL*MAP_VOXELS_PER_BLOCK
#define VERTEX_ELEMENT_COUNT 6
#define SIZEOF_VERTEX        sizeof(int8[VERTEX_ELEMENT_COUNT])

#define VERTEX_WIDTH       (MAP_BLOCK_WIDTH + 1)
#define VERTEX_HEIGHT      (MAP_BLOCK_HEIGHT + 1)
#define VERTEX_DEPTH       (MAP_BLOCK_DEPTH + 1)
#define VERTICES_PER_LAYER (VERTEX_WIDTH*VERTEX_HEIGHT)
#define VERTICES_PER_BLOCK VERTEX_WIDTH*VERTEX_HEIGHT*VERTEX_DEPTH

struct PushData {
	ivec4s selection;
	int i;
};

static void remesh_block(int b);
static void mesh_quad(int16* inds, int x1, int y1, int z1, int x2, int y2, int z2, int axis);
inline static bool is_visible(int x, int y, int z, int axis);

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

struct MapData     map_data;
struct VoxelBlock* map_blocks;

static struct Pipeline pipeln;
static UBO  ubo_bufs[2];
static VBO  vbo_buf;
static IBO* ibo_bufs;
static int* indcs;
static int blockc;
static ivec4s selections[MAP_MAX_SELECTIONS];
static int    selectionc;

void map_init(VkRenderPass render_pass)
{
	map_data.block_size = (ivec4s){ MAP_BLOCK_WIDTH, MAP_BLOCK_HEIGHT, MAP_BLOCK_DEPTH };
	ubo_bufs[0] = ubo_new(sizeof(map_data)); /* camera matrix, block dimensions and map dimensions */
	ubo_bufs[1] = ubo_new(sizeof(selections)); /* Selections to be highlighted */
	pipeln = (struct Pipeline){
		.vshader    = create_shader(SHADER_DIR "map.vert"),
		.fshader    = create_shader(SHADER_DIR "map.frag"),
		.vertbindc  = 1,
		.vertbinds  = vert_binds,
		.vertattrc  = ARRAY_LEN(vertex_attrs),
		.vertattrs  = vertex_attrs,
		.uboc       = 2,
		.ubos       = ubo_bufs,
		.pushstages = VK_SHADER_STAGE_VERTEX_BIT,
		.pushsz     = sizeof(int), /* Index of the current block being drawn */
	};
	pipeln_init(&pipeln, render_pass);
	DEBUG(3, "[MAP] Map initialized. Block dimensions are set to: %dx%dx%d",
	      MAP_BLOCK_WIDTH, MAP_BLOCK_HEIGHT, MAP_BLOCK_DEPTH);
}

// TODO: need to free old data on subsequent calls
void map_new(ivec3s dim)
{
	map_data.map_size = (ivec4s){
		ceil((float)dim.x/(float)MAP_BLOCK_WIDTH),
		ceil((float)dim.y/(float)MAP_BLOCK_HEIGHT),
		ceil((float)dim.z/(float)MAP_BLOCK_DEPTH),
	};
	blockc = map_data.map_size.x*map_data.map_size.y*map_data.map_size.z;

	map_blocks = scalloc(blockc, sizeof(struct VoxelBlock));
	for (int i = 0; i < blockc; i++) {
		// map_blocks[i].voxels = scalloc(MAP_VOXELS_PER_BLOCK, sizeof(struct Voxel));
		for (int z = 0; z < MAP_BLOCK_DEPTH; z++)
			for (int y = 0; y < MAP_BLOCK_HEIGHT; y++)
				for (int x = 0; x < MAP_BLOCK_WIDTH; x++)
					map_get_voxel((ivec3s){ x, y, z })->data = 1;
					// map_get_voxel((ivec3s){ x, y, z })->data = x == 0 || x == MAP_BLOCK_WIDTH-1 || y == 0 || y == MAP_BLOCK_HEIGHT-1? 1: 0;
	}

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
	indcs    = scalloc(blockc, sizeof(*indcs));
	for (int b = 0; b < blockc; b++)
		remesh_block(b);

	DEBUG(3, "[MAP] Initialized map with dimensions: %dx%dx%d (%d blocks)",
	      map_data.map_size.x*MAP_BLOCK_WIDTH, map_data.map_size.y*MAP_BLOCK_HEIGHT, map_data.map_size.z*MAP_BLOCK_DEPTH, blockc);
}

int map_highlight_area(ivec4s area)
{
	selections[selectionc] = area;

	return selectionc++;
}

void map_clear_highlight()
{
	selectionc = 0;
	memset(selections, 0, sizeof(selections));
}

void map_record_commands(VkCommandBuffer cmd_buf)
{
	camera_get_vp(map_data.cam_vp);
	buffer_update(ubo_bufs[0], sizeof(map_data), &map_data);
	buffer_update(ubo_bufs[1], sizeof(selections), selections);

	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, &pipeln.dset, 0, NULL);

	vkCmdBindVertexBuffers(cmd_buf, 0, 1, &vbo_buf.buf, (VkDeviceSize[]){ 0 });
	for (int i = 0; i < blockc; i++) {
		// DEBUG(1, "[%d] Drawing %d vertices", i, models[i].meshes[m].vertc);
		vkCmdBindIndexBuffer(cmd_buf, ibo_bufs[i].buf, 0, VK_INDEX_TYPE_UINT16);
		vkCmdPushConstants(cmd_buf, pipeln.layout, pipeln.pushstages, 0, pipeln.pushsz, &i);
		vkCmdDrawIndexed(cmd_buf, indcs[i], 1, 0, 0, 0);
	}
}

void map_free()
{
	vbo_free(&vbo_buf);
	for (int i = 0; i < blockc; i++)
		ibo_free(&ibo_bufs[i]);

	free(ibo_bufs);
	free(indcs);
	// for (int i = 0; i < blockc; i++)
		// if (map_blocks[i].voxels)
			// free(map_blocks[i].voxels);
	free(map_blocks);

	pipeln_free(&pipeln);
}

static void remesh_block(int b)
{
	intptr inds_size = 3*TRIANGLES_PER_BLOCK*sizeof(uint16);
	int16* inds_data = smalloc(inds_size);
	int16* inds = inds_data;
	int indc = 0;

	// TODO: greed over multiple rows as well
	uint16 current_vxl, next_vxl;
	int x0 = 0, y0 = 0;
	for (int z = 0; z < MAP_BLOCK_DEPTH; z++) {
		/* Tops */
		for (int y = 0; y < MAP_BLOCK_HEIGHT; y++) {
			for (int x = 0; x < MAP_BLOCK_WIDTH; x++) {
				x0 = x;
				y0 = y;
				if (!(current_vxl = map_get_voxel((ivec3s){ x, y, z })->data) || !is_visible(x, y, z, 2))
					continue;

				next_vxl = current_vxl;
				while (x < MAP_BLOCK_WIDTH) {
					next_vxl = map_get_voxel((ivec3s){ x, y, z })->data;
					if (next_vxl != current_vxl)
						break;
					x++;
				}

				mesh_quad(inds, x0, y0, z, x, y + 1, z + 1, 2);
				inds += 6;
				indc += 6;
			}
		}
		/* Right sides */
		for (int y = 0; y < MAP_BLOCK_HEIGHT; y++) {
			for (int x = 0; x < MAP_BLOCK_WIDTH; x++) {
				x0 = x;
				y0 = y;
				if (!(current_vxl = map_get_voxel((ivec3s){ x, y, z })->data) || !is_visible(x, y, z, 1))
					continue;

				next_vxl = current_vxl;
				while (x < MAP_BLOCK_WIDTH) {
					next_vxl = map_get_voxel((ivec3s){ x, y, z })->data;
					if (next_vxl != current_vxl)
						break;
					x++;
				}

				mesh_quad(inds, x0, y0, z, x, y + 1, z + 1, 1);
				inds += 6;
				indc += 6;
			}
		}
		/* Left sides */
		for (int x = 0; x < MAP_BLOCK_WIDTH; x++) {
			for (int y = 0; y < MAP_BLOCK_HEIGHT; y++) {
				x0 = x;
				y0 = y;
				if (!(current_vxl = map_get_voxel((ivec3s){ x, y, z })->data) || !is_visible(x, y, z, 0))
					continue;

				next_vxl = current_vxl;
				while (y < MAP_BLOCK_HEIGHT) {
					next_vxl = map_get_voxel((ivec3s){ x, y, z })->data;
					if (next_vxl != current_vxl)
						break;
					y++;
				}

				mesh_quad(inds, x0, y0, z, x + 1, y, z + 1, 0);
				inds += 6;
				indc += 6;
			}
		}
	}

	ibo_bufs[b] = ibo_new(inds_size, inds_data);
	indcs[b]    = indc;
	free(inds_data);
}

/* 0 = x-axis; 1 = y-axis; 2 = z-axis */
#define VERTEX_INDEX(x, y, z, axis) ((z)*VERTICES_PER_LAYER + (y)*VERTEX_WIDTH + (x) + axis*VERTICES_PER_BLOCK)
static void mesh_quad(int16* inds, int x1, int y1, int z1, int x2, int y2, int z2, int axis)
{
	// DEBUG(1, "Meshing: %d, %d, %d to %d, %d, %d", x1, y1, z1, x2, y2, z2);
	switch (axis) {
		case 0: /* Left side triangles */
			*inds++ = VERTEX_INDEX(x2, y2, z1, 0);
			*inds++ = VERTEX_INDEX(x2, y2, z2, 0);
			*inds++ = VERTEX_INDEX(x2, y1, z1, 0);
			*inds++ = VERTEX_INDEX(x2, y1, z1, 0);
			*inds++ = VERTEX_INDEX(x2, y2, z2, 0);
			*inds++ = VERTEX_INDEX(x2, y1, z2, 0);
			break;
		case 1: /* Right side triangles */
			*inds++ = VERTEX_INDEX(x1, y2, z1, 1);
			*inds++ = VERTEX_INDEX(x1, y2, z2, 1);
			*inds++ = VERTEX_INDEX(x2, y2, z1, 1);
			*inds++ = VERTEX_INDEX(x2, y2, z1, 1);
			*inds++ = VERTEX_INDEX(x1, y2, z2, 1);
			*inds++ = VERTEX_INDEX(x2, y2, z2, 1);
			break;
		case 2: /* Top triangles */
			*inds++ = VERTEX_INDEX(x1, y1, z1, 2);
			*inds++ = VERTEX_INDEX(x1, y2, z1, 2);
			*inds++ = VERTEX_INDEX(x2, y1, z1, 2);
			*inds++ = VERTEX_INDEX(x2, y1, z1, 2);
			*inds++ = VERTEX_INDEX(x1, y2, z1, 2);
			*inds++ = VERTEX_INDEX(x2, y2, z1, 2);
			break;
		default:
			ERROR("[MAP] Invalid axis value: %d", axis);
	}
}

inline static bool is_visible(int x, int y, int z, int axis)
{
	if (!map_get_voxel((ivec3s){ x, y, z })->data)
		return false;

	switch (axis) {
		case 0: /* x-axis */
			if (x + 1 >= MAP_BLOCK_WIDTH)
				return true;
			else
				return map_get_voxel((ivec3s){ x + 1, y, z })->data == 0;
		case 1: /* y-axis */
			if (y + 1 >= MAP_BLOCK_HEIGHT)
				return true;
			else
				return map_get_voxel((ivec3s){ x, y + 1, z })->data == 0;
		case 2: /* z-axis */
			if (z == 0)
				return true;
			else
				return map_get_voxel((ivec3s){ x, y, z - 1 })->data == 0;
		default: // TODO: some sort of ray cast check for diagonals?
			return true;
	}
}
