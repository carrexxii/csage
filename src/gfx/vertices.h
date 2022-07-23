#ifndef GFX_VERTICES_H
#define GFX_VERTICES_H

#include <vulkan/vulkan.h>

#include "buffers.h"

#define SIZEOF_VERTEX sizeof(float[6])

static VkVertexInputBindingDescription vertbinds[] = {
	/* xyzrgb */
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
};

static VBO trivbo;
static uint32 trivertc = 3;
static float triverts[] = {
/*    x     y    z    r    g    b   */
	 0.0, -0.5, 0.0, 0.0, 1.0, 0.0,
	 0.5,  0.5, 0.0, 0.0, 0.0, 1.0,
	-0.5,  0.5, 0.0, 1.0, 0.0, 0.0,
};

#endif
