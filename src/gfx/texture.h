#ifndef GFX_TEXTURE_H
#define GFX_TEXTURE_H

#include <vulkan/vulkan.h>

struct Texture {
	VkImage        img;
	VkImageView    imgview;
	VkDeviceMemory mem;
};

struct Texture texture_new(uint8* pxs, int w, int h);
struct Texture texture_new_from_image(const char* path);
void texture_free(struct Texture tex);

#endif
