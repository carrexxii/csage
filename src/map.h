#ifndef MAP_H
#define MAP_H

#include "vulkan/vulkan.h"

#include "camera.h"

#define MAP_MAX_VOXEL_SELECTIONS 16

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

void map_init(VkRenderPass renderpass, struct Camera* cam);
void map_new(Vec3i dim);
void map_mouse_select(bool kdown);
void map_mouse_deselect(bool kdown);
int  map_highlight_area(Recti area);
void map_clear_highlight(void);
void map_update(void);
void map_record_commands(VkCommandBuffer cmd_buf, struct Camera* cam);
void map_free(void);

extern struct MapData {
	Mat4x4 proj;
	Mat4x4 view;
	Vec3i map_size;
	float pad1;
	Vec3i block_size;
	float pad2;
} map_data;
extern struct VoxelBlock* map_blocks;

inline static int map_get_block_x(Vec3i world_pos) { return world_pos.x / MAP_BLOCK_WIDTH;  }
inline static int map_get_block_y(Vec3i world_pos) { return world_pos.y / MAP_BLOCK_HEIGHT; }
inline static int map_get_block_z(Vec3i world_pos) { return world_pos.z / MAP_BLOCK_DEPTH;  }
inline static struct VoxelBlock* map_get_block(Vec3i world_pos) {
	return &map_blocks[map_get_block_z(world_pos)*MAP_BLOCKS_PER_LAYER +
	                   map_get_block_y(world_pos)*MAP_BLOCK_WIDTH      +
	                   map_get_block_x(world_pos)];
}

inline static int map_get_voxel_x(Vec3i local_pos) { return local_pos.x % MAP_BLOCK_WIDTH;  }
inline static int map_get_voxel_y(Vec3i local_pos) { return local_pos.y % MAP_BLOCK_HEIGHT; }
inline static int map_get_voxel_z(Vec3i local_pos) { return local_pos.z % MAP_BLOCK_DEPTH;  }
inline static struct Voxel* map_get_voxel(Vec3i world_pos) {
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
