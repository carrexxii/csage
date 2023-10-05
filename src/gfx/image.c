#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "vulkan.h"
#include "device.h"
#include "buffers.h"
#include "swapchain.h"
#include "image.h"

int32           imagec;
VkImage*        images;
VkImageView*    imageviews;
VkDeviceMemory* imagemems;

VkImage        depthimg;
VkImageView    depthview;
VkDeviceMemory depthmem;
VkFormat       depthfmt;
VkImageLayout  depthlayout;

VkSampler sampler;

static void create_sampler(VkSampler* samp);

/* Requires that the images have been allocated by the swapchain */
void image_init()
{
	/* IMPROVEMENT: one block of memory */
	images     = smalloc(MAX_IMAGES*sizeof(VkImage));
	imageviews = smalloc(MAX_IMAGES*sizeof(VkImageView));
	imagemems  = smalloc(MAX_IMAGES*sizeof(VkDeviceMemory));
	create_sampler(&sampler);
}

void image_new(uint32 w, uint32 h, VkFormat fmt, VkImageAspectFlags asp)
{
	/* IMPROVEMENT: caching */
	VkImageCreateInfo imgi = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext  = NULL,
		.flags  = 0,
		.format = fmt,
		.extent = (VkExtent3D){
			.width  = w,
			.height = h,
			.depth  = 1,
		},
		.imageType     = VK_IMAGE_TYPE_2D,
		.tiling        = VK_IMAGE_TILING_OPTIMAL,
		.sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.mipLevels     = 1,
		.arrayLayers   = 1,
		.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		                 VK_IMAGE_USAGE_SAMPLED_BIT      |
		                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.samples               = VK_SAMPLE_COUNT_1_BIT,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices   = NULL,
	};
	if (vkCreateImage(gpu, &imgi, alloccb, &images[imagec]) != VK_SUCCESS)
		ERROR("[VK] Failed to create image");
	else
		DEBUG(3, "[VK] Created image (%ux%d)", w, h);

	VkMemoryRequirements memreq;
	vkGetImageMemoryRequirements(gpu, images[imagec], &memreq);
	VkMemoryAllocateInfo alloci = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize  = memreq.size,
		.memoryTypeIndex = find_memory_index(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
	};
	if (vkAllocateMemory(gpu, &alloci, alloccb, &imagemems[imagec]) != VK_SUCCESS)
		ERROR("[VK] Failed to allocate memory for image");
	vkBindImageMemory(gpu, images[imagec], imagemems[imagec], 0);

	imagec++;
}

VkImageView image_new_view(VkImage img, VkFormat fmt, VkImageAspectFlags asp)
{
	VkImageView imgview;
	VkImageViewCreateInfo viewi = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.image    = img,
		.format   = fmt,
		.components = {
			.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.a = VK_COMPONENT_SWIZZLE_IDENTITY,
		},
		.subresourceRange = {
			.aspectMask     = asp,
			.baseMipLevel   = 0,
			.levelCount     = 1,
			.baseArrayLayer = 0,
			.layerCount     = 1,
		},
	};
	if (vkCreateImageView(gpu, &viewi, alloccb, &imgview) != VK_SUCCESS)
		ERROR("[VK] Failed to create image view");
	else
		DEBUG(3, "[VK] Created image view");

	return imgview;
}

/* This will also create the image view and transition the layout */
void image_new_depth_image()
{
	VkImageCreateInfo imgi = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.format = VK_FORMAT_D16_UNORM,
		.extent = (VkExtent3D){
			.width = swapchainext.width,
			.height = swapchainext.height,
			.depth = 1,
		},
		.imageType = VK_IMAGE_TYPE_2D,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.mipLevels = 1,
		.arrayLayers = 1,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
		         VK_IMAGE_USAGE_SAMPLED_BIT,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = NULL,
	};
	if (vkCreateImage(gpu, &imgi, alloccb, &depthimg) != VK_SUCCESS)
		ERROR("[VK] Failed to create depth image");
	else
		DEBUG(3, "[VK] Created depth image");

	VkMemoryRequirements memreq;
	vkGetImageMemoryRequirements(gpu, depthimg, &memreq);
	VkMemoryAllocateInfo alloci = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memreq.size,
		.memoryTypeIndex = find_memory_index(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
	};
	if (vkAllocateMemory(gpu, &alloci, alloccb, &depthmem) != VK_SUCCESS)
		ERROR("[VK] Failed to allocate memory for depth image");
	vkBindImageMemory(gpu, depthimg, depthmem, 0);

	/* IMPROVEMENT: generalize into create_image_view */
	/* image view */
	VkImageViewCreateInfo viewi = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.image = depthimg,
		.format = VK_FORMAT_D16_UNORM,
		.components = {
			.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.a = VK_COMPONENT_SWIZZLE_IDENTITY,
		},
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};
	if (vkCreateImageView(gpu, &viewi, alloccb, &depthview) != VK_SUCCESS)
		ERROR("[VK] Failed to create depth image view");
	else
		DEBUG(3, "[VK] Created depth image view");

	/* transition layout */
	VkCommandBuffer buf = begin_command_buffer();
	VkImageMemoryBarrier bar = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = depthimg,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
		.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
	};
	vkCmdPipelineBarrier(buf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	                     0, 0, NULL, 0, NULL, 1, &bar);
	end_command_buffer(buf, NULL);
}

void image_transition_layout(VkImage img, VkImageLayout old, VkImageLayout new)
{
	VkCommandBuffer buf = begin_command_buffer();
	VkImageMemoryBarrier bar = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout           = old,
		.newLayout           = new,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image               = img,
		.subresourceRange = {
			.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel   = 0,
			.levelCount     = 1,
			.baseArrayLayer = 0,
			.layerCount     = 1,
		},
	};

	#define NO_LAYOUT_ERROR do {                                                               \
			ERROR("[VK] No layout transition implemented (line: %u). \n\t[old] %s\n\t[new] %s", \
			__LINE__, STRING_IMAGE_LAYOUT(old), STRING_IMAGE_LAYOUT(new));                       \
		} while (0)
	VkPipelineStageFlags dst = 0, src = 0;
	DEBUG_VALUE(STRING_IMAGE_LAYOUT(old));
	switch (old) {
		case VK_IMAGE_LAYOUT_UNDEFINED:
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			switch (new) {
				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
					bar.srcAccessMask = 0;
					bar.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					src = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
					dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
					break;
				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
					bar.srcAccessMask = 0;
					bar.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					src = VK_PIPELINE_STAGE_TRANSFER_BIT;
					dst = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					break;
				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
					bar.srcAccessMask = 0;
					bar.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
						                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
					src = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
					dst = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
					break;
				default:
					NO_LAYOUT_ERROR;
			}
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			switch (new) {
				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
					bar.srcAccessMask = VK_IMAGE_LAYOUT_UNDEFINED;
					bar.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					src = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
					dst = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					break;
				default:
					NO_LAYOUT_ERROR;
			}
			break;
		default:
			NO_LAYOUT_ERROR;
	}

	vkCmdPipelineBarrier(buf, src, dst, 0, 0, NULL, 0, NULL, 1, &bar);
	end_command_buffer(buf, NULL);
	DEBUG(3, "[VK] Image layout transitioned (%s -> %s)",
	      STRING_IMAGE_LAYOUT(old), STRING_IMAGE_LAYOUT(new));
}

void image_free()
{
	DEBUG(3, "[VK] Destroying image views...");
	for (int i = 0; i < imagec; i++) {
		vkDestroyImageView(gpu, imageviews[i], alloccb);
		vkDestroyImage(gpu, images[i], alloccb);
		if (imagemems[i])
			vkFreeMemory(gpu, imagemems[i], alloccb);
	}
	vkDestroyImageView(gpu, depthview, alloccb);
	vkDestroyImage(gpu, depthimg, alloccb);
	vkFreeMemory(gpu, depthmem, alloccb);

	DEBUG(3, "[VK] Destroying sampler...");
	vkDestroySampler(gpu, sampler, alloccb);

	free(images);
}

/* TODO: either add customization or remove parameter */
static void create_sampler(VkSampler* samp)
{
	VkSamplerCreateInfo spli = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.flags            = 0,
		.magFilter        = VK_FILTER_NEAREST,
		.minFilter        = VK_FILTER_NEAREST,
		.addressModeU     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.mipLodBias       = 0.0,
		.minLod           = 0.0,
		.maxLod           = 0.0,
		.anisotropyEnable = VK_FALSE,
		.maxAnisotropy    = 1,
		.compareEnable    = VK_FALSE,
		.compareOp        = VK_COMPARE_OP_ALWAYS,
		.borderColor      = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
	};
	if (vkCreateSampler(gpu, &spli, alloccb, samp) != VK_SUCCESS)
		ERROR("[VK] Failed to create sampler");
	else
		DEBUG(2, "[VK] Created sampler");
}
