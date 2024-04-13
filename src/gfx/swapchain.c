#include <vulkan/vulkan.h>

#include "vulkan.h"
#include "device.h"
#include "image.h"
#include "swapchain.h"

Swapchain        swapchain;
SwapchainDetails swapchain_details;
Image            depth_img;
Image            resolve_img;

static VkSurfaceFormatKHR choose_surface_format();
static VkPresentModeKHR   choose_present_mode();

// __attribute__((no_sanitize_address))
void swapchain_init(VkSurfaceKHR surf, int w, int h)
{
	swapchain.ext  = (VkExtent2D){ w, h };
	swapchain.fmt  = choose_surface_format();
	swapchain.mode = choose_present_mode();

	uint32 min_imgc = swapchain_details.abilities.minImageCount + 1;
	if (swapchain_details.abilities.maxImageCount > 0 && min_imgc > swapchain_details.abilities.maxImageCount)
		min_imgc = swapchain_details.abilities.maxImageCount;

	VkSwapchainCreateInfoKHR swapchaini = {
		.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface          = surf,
		.minImageCount    = min_imgc,
		.imageFormat      = swapchain.fmt.format,
		.imageColorSpace  = swapchain.fmt.colorSpace,
		.imageArrayLayers = 1,
		.imageExtent      = swapchain.ext,
		.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform     = swapchain_details.abilities.currentTransform,
		.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode      = swapchain.mode,
		.oldSwapchain     = NULL,
		.clipped          = true,
	};
	if (qinds.graphics != qinds.present) {
		INFO(TERM_DARK_GREEN "[VK] Using different queues for graphics and presenting");
		swapchaini.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		swapchaini.queueFamilyIndexCount = 2;
		swapchaini.pQueueFamilyIndices   = (uint32[]){ qinds.graphics, qinds.present };
	} else {
		INFO(TERM_DARK_GREEN "[VK] Using the same queues for graphics and presenting");
		swapchaini.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
		swapchaini.queueFamilyIndexCount = 0;
		swapchaini.pQueueFamilyIndices   = NULL;
	}

	if ((vk_err = vkCreateSwapchainKHR(logical_gpu, &swapchaini, NULL, &swapchain.swapchain)))
		ERROR("[VK] Failed to create swapchain:\n\t\"%s\"", STRING_OF_VK_RESULT(vk_err));
	else
		INFO(TERM_DARK_GREEN "[VK] Created swapchain\n\tQueue families: %u\n\tMinimum images: %u",
		      swapchaini.queueFamilyIndexCount, min_imgc);

	/*** -------------------- Create the Command Pool -------------------- ***/
	VkCommandPoolCreateInfo pooli = {
		.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.queueFamilyIndex = qinds.transfer,
		.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	};
	if (vkCreateCommandPool(logical_gpu, &pooli, NULL, &transfer_cmd_pool))
		ERROR("[VK] Failed to create command pool");
	else
		INFO(TERM_DARK_GREEN "[VK] Created command pool");

	pooli.queueFamilyIndex = qinds.graphics;
	if (vkCreateCommandPool(logical_gpu, &pooli, NULL, &graphics_cmd_pool))
		ERROR("[VK] Failed to create command pool");
	else
		INFO(TERM_DARK_GREEN "[VK] Created command pool");

	/*** -------------------- Create Swapchain Images -------------------- ***/
	vkGetSwapchainImagesKHR(logical_gpu, swapchain.swapchain, &swapchain.imgc, NULL);
	VkImage tmp_imgs[swapchain.imgc];
	swapchain.imgs = smalloc(swapchain.imgc*sizeof(Image));
	vkGetSwapchainImagesKHR(logical_gpu, swapchain.swapchain, &swapchain.imgc, tmp_imgs);
	for (int i = 0; i < (int)swapchain.imgc; i++) {
		swapchain.imgs[i].img  = tmp_imgs[i];
		swapchain.imgs[i].view = image_new_view(swapchain.imgs[i].img, swapchain.fmt.format, VK_IMAGE_ASPECT_COLOR_BIT);
		image_transition_layout(swapchain.imgs[i].img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}

	depth_img = image_new(swapchain.ext.width, swapchain.ext.height, VK_FORMAT_D16_UNORM,
	                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	                      gpu_properties.max_samples);
	depth_img.view = image_new_view(depth_img.img, VK_FORMAT_D16_UNORM, VK_IMAGE_ASPECT_DEPTH_BIT);
	image_transition_layout(depth_img.img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	resolve_img = image_new(swapchain.ext.width, swapchain.ext.height, swapchain.fmt.format,
	                        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	                        gpu_properties.max_samples);
	resolve_img.view = image_new_view(resolve_img.img, swapchain.fmt.format, VK_IMAGE_ASPECT_COLOR_BIT);
	image_transition_layout(resolve_img.img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void swapchain_set(VkPhysicalDevice dev, VkSurfaceKHR surf)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, surf, &swapchain_details.abilities);
	uint surf_fmtc;
	vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surf, &surf_fmtc, NULL);
	if (surf_fmtc != 0) {
		/* Check if enough memory has previously been allocated */
		if (surf_fmtc > swapchain_details.fmtc)
			swapchain_details.fmts = srealloc(swapchain_details.fmts, surf_fmtc * sizeof(VkSurfaceFormatKHR));
		vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surf, &surf_fmtc, swapchain_details.fmts);
	}
	swapchain_details.fmtc = surf_fmtc;

	uint present_modec;
	vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surf, &present_modec, NULL);
	if (present_modec != 0) {
		/* Check if enough memory has previously been allocated */
		if (present_modec > swapchain_details.modec)
			swapchain_details.modes = srealloc(swapchain_details.modes, present_modec * sizeof(VkPresentModeKHR));
		vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surf, &present_modec, swapchain_details.modes);
	}
	swapchain_details.modec = present_modec;
}

void swapchain_free()
{
	INFO(TERM_DARK_GREEN "[VK] Destroying swapchain image views...");
	for (int i = 0; i < (int)swapchain.imgc; i++)
		vkDestroyImageView(logical_gpu, swapchain.imgs[i].view, NULL);
	image_free(&depth_img);
	image_free(&resolve_img);

	INFO(TERM_DARK_GREEN "[VK] Destroying command pools...");
	vkDestroyCommandPool(logical_gpu, transfer_cmd_pool, NULL);
	vkDestroyCommandPool(logical_gpu, graphics_cmd_pool, NULL);

	INFO(TERM_DARK_GREEN "[VK] Destroying swapchain...");
	vkDestroySwapchainKHR(logical_gpu, swapchain.swapchain, NULL);

	free(swapchain.imgs);
}

/* -------------------------------------------------------------------- */

/* Function: choose_surface_format
 *   Find and return the most appropriate surface format from the
 *   array of available ones in `swpdetails.formats`. If a specific
 *   format is not found, the first format will be returned.
 */
static VkSurfaceFormatKHR choose_surface_format()
{
	uint fmtc = swapchain_details.fmtc;
	VkSurfaceFormatKHR* fmts = swapchain_details.fmts;

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
	for (int i = 0; i < (int)swapchain_details.modec; i++)
		switch (swapchain_details.modes[i]) {
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

