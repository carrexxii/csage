#ifndef GFX_DEVICE_H
#define GFX_DEVICE_H

#include <vulkan/vulkan.h>

extern VkPhysicalDevice physicalgpu;
extern VkDevice         gpu;

extern VkQueue graphicsq;
extern VkQueue presentq;
extern VkQueue transferq;

extern VkCommandPool cmdpool;

extern struct QueueFamilyIndices {
	int graphics;
	int present;
	int transfer;
} qinds;

void device_init_physical(VkInstance inst, VkSurfaceKHR surf);
void device_init_logical(VkSurfaceKHR surf);
void device_free();

#endif

