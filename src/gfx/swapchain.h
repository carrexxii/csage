#ifndef GFX_SWAPCHAIN_H
#define GFX_SWAPCHAIN_H

#include "image.h"

void swapchain_init(VkSurfaceKHR surf, uint w, uint h);
void swapchain_set(VkPhysicalDevice dev, VkSurfaceKHR surf);
void swapchain_free();

extern VkSwapchainKHR     swapchain;
extern VkExtent2D         swapchainext;
extern VkSurfaceFormatKHR surfacefmt;
extern VkPresentModeKHR   presentmd;

extern uint32       swapchainimgc;
extern VkImage*     swapchainimgs;
extern VkImageView* swapchainimgviews;

extern struct SwapchainDetails {
	VkSurfaceCapabilitiesKHR abilities;
	uint fmtc, mdc;
	VkSurfaceFormatKHR* fmts;
	VkPresentModeKHR* mds;
} swapdetails;

#endif
