#ifndef GFX_SWAPCHAIN_H
#define GFX_SWAPCHAIN_H

#include "common.h"
#include "image.h"

typedef struct Swapchain {
	VkSwapchainKHR     swapchain;
	VkExtent2D         ext;
	VkSurfaceFormatKHR fmt;
	VkPresentModeKHR   mode;
	Image*             imgs;
	uint               imgc;
} Swapchain;

typedef struct SwapchainDetails {
	VkSurfaceCapabilitiesKHR abilities;
	VkSurfaceFormatKHR* fmts;
	VkPresentModeKHR* modes;
	uint fmtc, modec;
} SwapchainDetails;

void swapchain_init(VkSurfaceKHR surf, int w, int h);
void swapchain_set(VkPhysicalDevice dev, VkSurfaceKHR surf);
void swapchain_free();

extern Swapchain        swapchain;
extern SwapchainDetails swapchain_details;
extern Image            depth_img;
extern Image            resolve_img;

#endif

