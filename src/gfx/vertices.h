#ifndef GFX_VERTICES_H
#define GFX_VERTICES_H

#include <vulkan/vulkan.h>

#include "buffers.h"

#define SIZEOF_VERTEX sizeof(float[5])

static VkVertexInputBindingDescription mdlvertbinds[] = {
	/* xyrgb */
	{ .binding   = 0,
	  .stride    = SIZEOF_VERTEX,
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
};
static VkVertexInputAttributeDescription mdlvertattrs[] = {
	/* xy */
	{ .binding  = 0,
	  .location = 0,
	  .format   = VK_FORMAT_R32G32_SFLOAT,
	  .offset   = 0, },
	/* rgb */
	{ .binding  = 0,
	  .location = 1,
	  .format   = VK_FORMAT_R32G32B32_SFLOAT,
	  .offset   = sizeof(float[2]), },
};

#endif
