#ifndef MAP_H
#define MAP_H

#include "vulkan/vulkan.h"

#define MAP_BLOCK_WIDTH      32
#define MAP_BLOCK_HEIGHT     MAP_BLOCK_WIDTH
#define MAP_BLOCK_DEPTH      16
#define MAP_BLOCKS_PER_LAYER (MAP_BLOCK_WIDTH*MAP_BLOCK_HEIGHT)

#define MAP_VOXELS_PER_LAYER MAP_BLOCK_WIDTH*MAP_BLOCK_HEIGHT
#define MAP_VOXELS_PER_BLOCK MAP_VOXELS_PER_LAYER*MAP_BLOCK_DEPTH

struct Voxel {
	uint16 data;
};

struct VoxelBlock {
	struct Voxel* voxels;
	int voxelc;
};

void map_init(VkRenderPass render_pass);
void map_new(ivec3s dim);
void map_record_commands(VkCommandBuffer cmd_buf);
void map_free();

extern struct MapData {
	mat4   cam_vp;
	ivec4s map_size;
	ivec4s block_size;
} map_data;
extern struct VoxelBlock* map_blocks;

[[gnu::always_inline]]
inline static int map_get_block_index(ivec3s pos) {
	return (pos.z/MAP_BLOCK_DEPTH)*map_data.map_size.z +
	       (pos.y/MAP_BLOCK_WIDTH)*map_data.map_size.y +
	       (pos.x/MAP_BLOCK_WIDTH);
}

[[gnu::always_inline]]
inline static struct Voxel* map_get_voxel(ivec3s pos) {
	struct VoxelBlock* block = &map_blocks[map_get_block_index(pos)];
	return &block->voxels[(pos.z % MAP_BLOCK_DEPTH)*MAP_VOXELS_PER_LAYER +
	                      (pos.y % MAP_BLOCK_HEIGHT)*MAP_BLOCK_WIDTH + 
	                      (pos.x % MAP_BLOCK_WIDTH)];
}

#endif
