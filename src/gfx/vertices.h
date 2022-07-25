#ifndef GFX_VERTICES_H
#define GFX_VERTICES_H

#include <vulkan/vulkan.h>

#include "buffers.h"

#define SIZEOF_VERTEX sizeof(float[9])

static VkVertexInputBindingDescription vertbinds[] = {
	/* xyzrgbnnn */
	{ .binding   = 0,
	  .stride    = SIZEOF_VERTEX,
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
};

static VkVertexInputAttributeDescription vertattrs[] = {
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

#endif
