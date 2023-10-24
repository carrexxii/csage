#ifndef MAP_H
#define MAP_H

#include "vulkan/vulkan.h"

#define MAP_MAX_SELECTIONS 64

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
	struct Voxel voxels[MAP_BLOCK_DEPTH][MAP_BLOCK_HEIGHT][MAP_BLOCK_WIDTH];
};

void map_init(VkRenderPass render_pass);
void map_new(ivec3s dim);
int  map_highlight_area(ivec4s area);
void map_record_commands(VkCommandBuffer cmd_buf);
void map_free();

extern struct MapData {
	mat4   cam_vp;
	ivec4s map_size;
	ivec4s block_size;
} map_data;
extern struct VoxelBlock* map_blocks;

inline static int map_get_block_x(ivec3s world_pos) { return world_pos.x / MAP_BLOCK_WIDTH;  }
inline static int map_get_block_y(ivec3s world_pos) { return world_pos.y / MAP_BLOCK_HEIGHT; }
inline static int map_get_block_z(ivec3s world_pos) { return world_pos.z / MAP_BLOCK_DEPTH;  }
inline static struct VoxelBlock* map_get_block(ivec3s world_pos) {
	return &map_blocks[map_get_block_z(world_pos)*MAP_BLOCKS_PER_LAYER +
	                   map_get_block_y(world_pos)*MAP_BLOCK_WIDTH      +
	                   map_get_block_x(world_pos)];
}

inline static int map_get_voxel_x(ivec3s local_pos) { return local_pos.x % MAP_BLOCK_WIDTH;  }
inline static int map_get_voxel_y(ivec3s local_pos) { return local_pos.y % MAP_BLOCK_HEIGHT; }
inline static int map_get_voxel_z(ivec3s local_pos) { return local_pos.z % MAP_BLOCK_DEPTH;  }
inline static struct Voxel* map_get_voxel(ivec3s world_pos) {
	int x = map_get_voxel_x(world_pos);
	int y = map_get_voxel_y(world_pos);
	int z = map_get_voxel_z(world_pos);
	if (BETWEEN(x, 0, MAP_BLOCK_WIDTH  - 1) &&
		BETWEEN(y, 0, MAP_BLOCK_HEIGHT - 1) &&
		BETWEEN(z, 0, MAP_BLOCK_DEPTH  - 1))
		return &map_get_block(world_pos)->voxels[z][y][x];
	else
		return NULL;
}

#endif
