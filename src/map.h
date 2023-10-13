#ifndef MAP_H
#define MAP_H

#include "vulkan/vulkan.h"

struct Voxel {
	uint16 data;
};

void map_init(VkRenderPass render_pass);
void map_new(ivec3s dim);
void map_record_commands(VkCommandBuffer cmd_buf);
void map_free();

#endif
