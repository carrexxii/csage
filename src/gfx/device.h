#ifndef GFX_DEVICE_H
#define GFX_DEVICE_H

#include <vulkan/vulkan.h>

void device_init_physical(VkInstance inst, VkSurfaceKHR surf);
void device_init_logical(VkSurfaceKHR surf);
int  device_find_memory_index(uint type, VkMemoryPropertyFlagBits prop);
void device_free();

extern VkDevice      logical_gpu;
extern VkCommandPool transfer_cmd_pool;
extern VkCommandPool graphics_cmd_pool;
extern VkQueue       graphicsq;
extern VkQueue       presentq;
extern VkQueue       transferq;
extern struct QueueFamilyIndices {
	int graphics;
	int present;
	int transfer;
} qinds;
extern struct DeviceLimits {
	VkSampleCountFlagBits max_samples;
	bool lazy_mem;
} gpu_properties;

#endif
