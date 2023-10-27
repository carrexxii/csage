#include <vulkan/vulkan.h>

#include "vulkan.h"
#include "device.h"
#include "image.h"
#include "swapchain.h"

VkSwapchainKHR     swapchain;
VkExtent2D         swapchainext;
VkSurfaceFormatKHR surfacefmt;
VkPresentModeKHR   presentmd;

uint32       swapchainimgc;
VkImage*     swapchainimgs;
VkImageView* swapchainimgviews;

struct SwapchainDetails swapdetails;

static void create_command_pool();
static VkSurfaceFormatKHR choose_surface_format();
static VkPresentModeKHR choose_present_mode();

void swapchain_init(VkSurfaceKHR surf, int w, int h)
{
	swapchainext = (VkExtent2D){ w, h };
	surfacefmt   = choose_surface_format();
	presentmd    = choose_present_mode();

	uint32 minimgc = swapdetails.abilities.minImageCount + 1;
	if (swapdetails.abilities.maxImageCount > 0 &&
		minimgc > swapdetails.abilities.maxImageCount)
		minimgc = swapdetails.abilities.maxImageCount;

	VkSwapchainCreateInfoKHR swapchaini = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface          = surf,
		.minImageCount    = minimgc,
		.imageFormat      = surfacefmt.format,
		.imageColorSpace  = surfacefmt.colorSpace,
		.imageArrayLayers = 1,
		.imageExtent      = swapchainext,
		.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform     = swapdetails.abilities.currentTransform,
		.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode      = presentmd,
		.oldSwapchain     = NULL,
		.clipped          = true,
	};
	if (qinds.graphics != qinds.present) {
		DEBUG(3, "[VK] Using different queues for graphics and presenting");
		swapchaini.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		swapchaini.queueFamilyIndexCount = 2;
		swapchaini.pQueueFamilyIndices   = (uint32[]){ qinds.graphics, qinds.present };
	} else {
		DEBUG(3, "[VK] Using the same queues for graphics and presenting");
		swapchaini.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
		swapchaini.queueFamilyIndexCount = 0;
		swapchaini.pQueueFamilyIndices   = NULL;
	}

	if (vkCreateSwapchainKHR(gpu, &swapchaini, NULL, &swapchain) != VK_SUCCESS)
		ERROR("[VK] Failed to create swapchain");
	else
		DEBUG(1, "[VK] Created swapchain (queue families: %u | minimum images: %u)",
		      swapchaini.queueFamilyIndexCount, minimgc);

	create_command_pool();

	image_init(); 
	vkGetSwapchainImagesKHR(gpu, swapchain, &swapchainimgc, NULL);
	/* IMPROVEMENT: combine allocations */
	swapchainimgs     = smalloc(swapchainimgc*sizeof(VkImage));
	swapchainimgviews = smalloc(swapchainimgc*sizeof(VkImageView));
	vkGetSwapchainImagesKHR(gpu, swapchain, &swapchainimgc, swapchainimgs);
	for (int i = 0; i < (int)swapchainimgc; i++) {
		swapchainimgviews[i] = image_new_view(swapchainimgs[i], surfacefmt.format, VK_IMAGE_ASPECT_COLOR_BIT);
		image_transition_layout(swapchainimgs[i], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}
	image_new_depth_image();
}

void swapchain_set(VkPhysicalDevice dev, VkSurfaceKHR surf)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, surf, &swapdetails.abilities);
	uint surffmtc;
	vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surf, &surffmtc, NULL);
	if (surffmtc != 0) {
		/* Check if enough memory has previously been allocated */
		if (surffmtc > swapdetails.fmtc)
			swapdetails.fmts = srealloc(swapdetails.fmts, surffmtc * sizeof(VkSurfaceFormatKHR));
		vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surf, &surffmtc, swapdetails.fmts);
	}
	swapdetails.fmtc = surffmtc;

	uint surfpresmdc;
	vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surf, &surfpresmdc, NULL);
	if (surfpresmdc != 0) {
		/* Check if enough memory has previously been allocated */
		if (surfpresmdc > swapdetails.mdc)
			swapdetails.mds = srealloc(swapdetails.mds, surfpresmdc * sizeof(VkPresentModeKHR));
		vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surf, &surfpresmdc, swapdetails.mds);
	}
	swapdetails.mdc = surfpresmdc;
}

void swapchain_free()
{
	DEBUG(3, "[VK] Destroying swapchain image views...");
	for (int i = 0; i < (int)swapchainimgc; i++)
		vkDestroyImageView(gpu, swapchainimgviews[i], NULL);
	DEBUG(3, "[VK] Destroying command pool...");
	vkDestroyCommandPool(gpu, cmdpool, NULL);
	DEBUG(3, "[VK] Destroying swapchain...");
	vkDestroySwapchainKHR(gpu, swapchain, NULL);

	free(swapchainimgs);
}

static void create_command_pool()
{
	VkCommandPoolCreateInfo pooli = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.queueFamilyIndex = qinds.graphics,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	};
	if (vkCreateCommandPool(gpu, &pooli, NULL, &cmdpool) != VK_SUCCESS)
		ERROR("[VK] Failed to create command pool");
	else
		DEBUG(3, "[VK] Created command pool");
}

/* Function: choose_surface_format
 *   Find and return the most appropriate surface format from the
 *   array of available ones in `swpdetails.formats`. If a specific
 *   format is not found, the first format will be returned.
 */
static VkSurfaceFormatKHR choose_surface_format()
{
	uint fmtc = swapdetails.fmtc;
	VkSurfaceFormatKHR* fmts = swapdetails.fmts;

	/* Device has no preferred format */
	if (fmtc == 1 && fmts[0].format == VK_FORMAT_UNDEFINED)
		return (VkSurfaceFormatKHR) {
			.format = VK_FORMAT_R8G8B8A8_UNORM,
			.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
	};
	/* TODO: actually find the best available format */
	for (int i = 0; i < (int)fmtc; i++)
		if (fmts[i].format == VK_FORMAT_R8G8B8A8_UNORM &&
			fmts[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return fmts[i];

	return fmts[0];
}

VkPresentModeKHR choose_present_mode()
{
	VkPresentModeKHR md = VK_PRESENT_MODE_IMMEDIATE_KHR;
	for (int i = 0; i < (int)swapdetails.mdc; i++)
		switch (swapdetails.mds[i]) {
		case VK_PRESENT_MODE_MAILBOX_KHR:
			return VK_PRESENT_MODE_MAILBOX_KHR;
		case VK_PRESENT_MODE_FIFO_KHR:
			md = VK_PRESENT_MODE_FIFO_KHR;
			break;
		default:
			break;
		}

	return md;
}
