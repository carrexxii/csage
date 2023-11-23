#ifndef GFX_DEVICE_H
#define GFX_DEVICE_H

#include <vulkan/vulkan.h>

void device_init_physical(VkInstance inst, VkSurfaceKHR surf);
void device_init_logical(VkSurfaceKHR surf);
int  device_find_memory_index(uint type, VkMemoryPropertyFlagBits prop);
int  device_get_max_sample_count(VkPhysicalDevice dev);
void device_free();

extern VkDevice      logical_gpu;
extern VkCommandPool cmd_pool;
extern VkQueue       graphicsq;
extern VkQueue       presentq;
extern VkQueue       transferq;
extern struct QueueFamilyIndices {
	int graphics;
	int present;
	int transfer;
} qinds;

#endif
