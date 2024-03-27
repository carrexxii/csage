#include "vulkan.h"

#include "image.h"
#include "texture.h"

static void buffer_to_image(VkBuffer buf, VkImage img, uint w, uint h);

struct Texture texture_of_memory(int w, int h, uint8* pxs)
{
	struct Texture tex;

	void* data;
	struct Buffer trans_buf;
	VkDeviceSize size = 4 * w * h;
	buffer_new(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	           &trans_buf.buf, &trans_buf.mem);
	vkMapMemory(logical_gpu, trans_buf.mem, 0, size, 0, &data);
	memcpy(data, pxs, size);
	vkUnmapMemory(logical_gpu, trans_buf.mem);

	struct Image img = image_new(w, h, VK_FORMAT_R8G8B8A8_UNORM,
	                             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	                             VK_SAMPLE_COUNT_1_BIT);

	image_transition_layout(img.img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	buffer_to_image(trans_buf.buf, img.img, w, h);
	image_transition_layout(img.img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	tex.image      = img.img;
	tex.memory     = img.mem;
	tex.image_view = image_new_view(img.img, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	buffer_free(&trans_buf);
	return tex;
}

void texture_free(struct Texture* tex)
{
	if (!tex->image || !tex->memory) {
		ERROR("[VK] Texture (%p -> %p img | %p mem) does not appear to be valid (probably uninitialized or double free)",
		      tex, tex->image, tex->memory);
		return;
	}

	vkDestroyImageView(logical_gpu, tex->image_view, NULL);
	vkDestroyImage(logical_gpu, tex->image, NULL);
	vkFreeMemory(logical_gpu, tex->memory, NULL);
	memset(tex, 0, sizeof(struct Texture));
}

/* -------------------------------------------------------------------- */

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
	end_command_buffer(cbuf, NULL);
}
