#ifndef GFX_SWAPCHAIN_H
#define GFX_SWAPCHAIN_H

#include "image.h"

struct Swapchain {
	VkSwapchainKHR     swapchain;
	VkExtent2D         ext;
	VkSurfaceFormatKHR fmt;
	VkPresentModeKHR   mode;
	struct Image*      imgs;
	uint               imgc;
};

struct SwapchainDetails {
	VkSurfaceCapabilitiesKHR abilities;
	VkSurfaceFormatKHR* fmts;
	VkPresentModeKHR* modes;
	uint fmtc, modec;
};

void swapchain_init(VkSurfaceKHR surf, int w, int h);
void swapchain_set(VkPhysicalDevice dev, VkSurfaceKHR surf);
void swapchain_free();

extern struct Swapchain        swapchain;
extern struct SwapchainDetails swapchain_details;
extern struct Image depth_img;
extern struct Image colour_img;

#endif
