#ifndef GFX_TEXTURE_H
#define GFX_TEXTURE_H

#include "vulkan.h"

struct Texture {
	VkImage        image;
	VkImageView    image_view;
	VkDeviceMemory memory;
};

struct Texture texture_of_memory(int w, int h, uint8* pxs);
void texture_free(struct Texture* tex);

#endif
