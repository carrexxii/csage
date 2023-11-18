#ifndef GFX_IMAGE_H
#define GFX_IMAGE_H

#include <vulkan/vulkan.h>

#define MAX_IMAGES 64

extern int32           imagec;
extern VkImage*        images;
extern VkImageView*    imageviews;
extern VkDeviceMemory* imagemems;

extern VkImage        depthimg;
extern VkImageView    depthview;
extern VkDeviceMemory depthmem;
extern VkFormat       depthfmt;
extern VkImageLayout  depthlayout;

extern VkSampler sampler;

void image_init();
void image_new(uint32 w, uint32 h, VkFormat fmt);
VkImageView image_new_view(VkImage img, VkFormat fmt, VkImageAspectFlags asp);
void image_new_depth_image();
void image_transition_layout(VkImage img, VkImageLayout old, VkImageLayout new);
void image_free();

#endif
