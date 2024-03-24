#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "vulkan/vulkan.h"

#include "vulkan.h"
#include "device.h"
#include "pipeline.h"
#include "buffers.h"
#include "image.h"
#include "texture.h"

static void buffer_to_image(VkBuffer buf, VkImage img, uint w, uint h);

// TODO: rename (new/load)
struct Texture texture_new(byte* pxs, int w, int h)
{
	DEBUG(2, "[VK] Creating new texture (%dx%d)", w, h);
	struct Texture tex;

	void* data;
	struct Buffer trans_buffer;
	VkDeviceSize size = 4*w*h;
	buffer_new(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	           &trans_buffer.buf, &trans_buffer.mem);
	vkMapMemory(logical_gpu, trans_buffer.mem, 0, size, 0, &data);
	memcpy(data, pxs, size);
	vkUnmapMemory(logical_gpu, trans_buffer.mem);

	// TODO: replace with image_new() et al
	VkImageCreateInfo imgi = {
		.sType  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext  = NULL,
		.flags  = 0,
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.extent = (VkExtent3D){
			.width  = (uint32)w,
			.height = (uint32)h,
			.depth  = 1,
		},
		.imageType             = VK_IMAGE_TYPE_2D,
		.tiling                = VK_IMAGE_TILING_OPTIMAL,
		.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
		.mipLevels             = 1,
		.arrayLayers           = 1,
		.usage                 = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.samples               = VK_SAMPLE_COUNT_1_BIT,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices   = NULL,
	};
	if (vkCreateImage(logical_gpu, &imgi, NULL, &tex.image) != VK_SUCCESS)
		ERROR("[VK] Failed to create image");

	VkMemoryRequirements mem_req;
	vkGetImageMemoryRequirements(logical_gpu, tex.image, &mem_req);
	VkMemoryAllocateInfo alloci = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize  = mem_req.size,
		.memoryTypeIndex = device_find_memory_index(mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
	};
	if (vkAllocateMemory(logical_gpu, &alloci, NULL, &tex.memory) != VK_SUCCESS)
		ERROR("[VK] Failed to allocate memory for image");
	vkBindImageMemory(logical_gpu, tex.image, tex.memory, 0);

	image_transition_layout(tex.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	buffer_to_image(trans_buffer.buf, tex.image, (uint)w, (uint)h);
	image_transition_layout(tex.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	tex.image_view = image_new_view(tex.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	buffer_free(&trans_buffer);

	return tex;
}

// TODO: rename to _from_file
struct Texture texture_new_from_image(const char* path)
{
	struct Texture tex;
	int w, h, ch;
	uint8* pxs = stbi_load(path, &w, &h, &ch, 4);
	if (!pxs)
		ERROR("[RES] Failed to load image \"%s\"", path);

	tex = texture_new(pxs, w, h);
	stbi_image_free(pxs);

	return tex;
}

void texture_free(struct Texture* tex)
{
	if (!tex->image || !tex->memory) {
		ERROR("[VK] Texture does not appear to be valid (probably uninitialized or double free)");
		return;
	}
	vkDestroyImageView(logical_gpu, tex->image_view, NULL);
	vkDestroyImage(logical_gpu, tex->image, NULL);
	vkFreeMemory(logical_gpu, tex->memory, NULL);
	memset(tex, 0, sizeof(struct Texture));
}

static void buffer_to_image(VkBuffer buf, VkImage img, uint w, uint h)
{
	VkCommandBuffer cbuf = begin_command_buffer();
	VkBufferImageCopy rgn = {
		.bufferOffset      = 0,
		.bufferRowLength   = 0,
		.bufferImageHeight = 0,
		.imageSubresource = {
			.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel       = 0,
			.baseArrayLayer = 0,
			.layerCount     = 1,
		},
		.imageOffset = { 0, 0, 0 },
		.imageExtent = { w, h, 1 },
	};
	vkCmdCopyBufferToImage(cbuf, buf, img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &rgn);
	/* TODO: Fence */
	end_command_buffer(cbuf, NULL);
}
