#ifndef GFX_IMAGE_H
#define GFX_IMAGE_H

#include <vulkan/vulkan.h>

#define MAX_IMAGES 64

struct Image {
	VkImage        img;
	VkImageView    view;
	VkDeviceMemory mem;
};

void image_init();
struct Image image_new(uint w, uint h, VkFormat fmt, VkImageUsageFlags usage, VkSampleCountFlags samples);
VkImageView image_new_view(VkImage img, VkFormat fmt, VkImageAspectFlags asp);
VkSampler image_new_sampler(VkFilter filter);
void image_transition_layout(VkImage img, VkImageLayout old, VkImageLayout new);
void image_free(struct Image* img);

#endif
