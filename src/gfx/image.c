#include <vulkan/vulkan.h>

#include "vulkan.h"
#include "device.h"
#include "buffers.h"
#include "swapchain.h"
#include "image.h"

struct Image image_new(uint w, uint h, VkFormat fmt, VkImageUsageFlags usage, VkSampleCountFlags samples)
{
	struct Image img;
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
		.imageType             = VK_IMAGE_TYPE_2D,
		.tiling                = VK_IMAGE_TILING_OPTIMAL,
		.sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
		.mipLevels             = 1,
		.arrayLayers           = 1,
		.usage                 = usage,
		.samples               = samples,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices   = NULL,
	};
	if (vkCreateImage(logical_gpu, &imgi, NULL, &img.img))
		ERROR("[VK] Failed to create image");
	else
		DEBUG(3, "[VK] Created image (%ux%d)", w, h);

	VkMemoryRequirements mem_req;
	vkGetImageMemoryRequirements(logical_gpu, img.img, &mem_req);
	VkMemoryAllocateInfo alloci = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize  = mem_req.size,
		.memoryTypeIndex = device_find_memory_index(mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | 
		                                            (gpu_properties.lazy_mem && samples? VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT: 0)),
	};
	if (vkAllocateMemory(logical_gpu, &alloci, NULL, &img.mem))
		ERROR("[VK] Failed to allocate memory for image");
	vkBindImageMemory(logical_gpu, img.img, img.mem, 0);

	return img;
}

VkImageView image_new_view(VkImage img, VkFormat fmt, VkImageAspectFlags asp)
{
	VkImageView img_view;
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
	if (vkCreateImageView(logical_gpu, &viewi, NULL, &img_view))
		ERROR("[VK] Failed to create image view");
	else
		DEBUG(3, "[VK] Created image view");

	return img_view;
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
		/* https://stackoverflow.com/questions/67993790/how-do-you-properly-transition-the-image-layout-from-transfer-optimal-to-shader */
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
	DEBUG(3, "[VK] Image layout transitioned (%s -> %s)", STRING_IMAGE_LAYOUT(old), STRING_IMAGE_LAYOUT(new));
}

void image_free(struct Image* img)
{
	vkDestroyImageView(logical_gpu, img->view, NULL);
	vkDestroyImage(logical_gpu, img->img, NULL);
	vkFreeMemory(logical_gpu, img->mem, NULL);
}

VkSampler image_new_sampler(VkFilter filter)
{
	VkSampler samp;
	VkSamplerCreateInfo sampi = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.flags            = 0,
		.magFilter        = filter,
		.minFilter        = filter,
		.addressModeU     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.mipLodBias       = 0.0,
		.minLod           = 0.0,
		.maxLod           = 0.0,
		.anisotropyEnable = false,
		.maxAnisotropy    = 1,
		.compareEnable    = false,
		.compareOp        = VK_COMPARE_OP_ALWAYS,
		.borderColor      = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = false,
	};
	if (vkCreateSampler(logical_gpu, &sampi, NULL, &samp))
		ERROR("[VK] Failed to create sampler");
	else
		DEBUG(2, "[VK] Created sampler");

	return samp;
}
