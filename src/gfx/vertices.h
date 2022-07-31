#ifndef GFX_VERTICES_H
#define GFX_VERTICES_H

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "buffers.h"

#define SIZEOF_MDL_VERT sizeof(float[9])
#define SIZEOF_VXL_VERT sizeof(float[3])

/* Models */
static VkVertexInputBindingDescription mdlvertbinds[] = {
	/* xyzrgbnnn */
	{ .binding   = 0,
	  .stride    = SIZEOF_MDL_VERT,
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
};
static VkVertexInputAttributeDescription mdlvertattrs[] = {
	/* xyz */
	{ .binding  = 0,
	  .location = 0,
	  .format   = VK_FORMAT_R32G32B32_SFLOAT,
	  .offset   = 0, },
	/* rgb */
	{ .binding  = 0,
	  .location = 1,
	  .format   = VK_FORMAT_R32G32B32_SFLOAT,
	  .offset   = sizeof(float[3]), },
	/* nnn */
	{ .binding  = 0,
	  .location = 2,
	  .format   = VK_FORMAT_R32G32B32_SFLOAT,
	  .offset   = sizeof(float[6]), },
};

/* Voxels */
static VkVertexInputBindingDescription vxlvertbinds[] = {
	{ .binding   = 0,
	  .stride    = SIZEOF_VXL_VERT,
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
};
static VkVertexInputAttributeDescription vxlvertattrs[] = {
	/* xyz */
	{ .binding  = 0,
	  .location = 0,
	  .format   = VK_FORMAT_R32G32B32_SFLOAT, /* TODO: find a better format that is supported */
	  .offset   = 0, },
};

#endif
