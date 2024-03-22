#include "vulkan.h"
#include "device.h"
#include "buffers.h"

static void copy_buffer(VkBuffer dst, VkBuffer src, VkDeviceSize s);

/* TODO: add multiple usage option */
VkCommandBuffer begin_command_buffer()
{
	VkCommandBuffer buf;
	VkCommandBufferAllocateInfo bufi = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandPool        = cmd_pool,
		.commandBufferCount = 1,
	};
	if ((vk_err = vkAllocateCommandBuffers(logical_gpu, &bufi, &buf)))
		ERROR("[VK] Failed to allocate command buffers\n\t\"%d\"", vk_err);

	VkCommandBufferBeginInfo begini = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	vkBeginCommandBuffer(buf, &begini);

	return buf;
}

void end_command_buffer(VkCommandBuffer buf, VkFence fence)
{
	vkEndCommandBuffer(buf);
	VkSubmitInfo submiti = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers    = &buf,
	};

	vkQueueSubmit(graphicsq, 1, &submiti, fence);
	if (!fence)
		vkQueueWaitIdle(graphicsq);
	vkFreeCommandBuffers(logical_gpu, cmd_pool, 1, &buf);
}

void buffer_new(VkDeviceSize sz, VkBufferUsageFlags buf_flags, VkMemoryPropertyFlags mem_flags, VkBuffer* buf, VkDeviceMemory* mem)
{
	VkBufferCreateInfo bufi = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size                  = sz,
		.usage                 = buf_flags,
		.flags                 = 0,
		.sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices   = NULL,
	};
	if ((vk_err = vkCreateBuffer(logical_gpu, &bufi, NULL, buf)))
		ERROR("\tFailed to create buffer\n\t\"%d\"", vk_err);

	VkMemoryRequirements mem_req;
	vkGetBufferMemoryRequirements(logical_gpu, *buf, &mem_req);
	VkMemoryAllocateInfo alloci = {
		.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize  = mem_req.size,
		.memoryTypeIndex = device_find_memory_index(mem_req.memoryTypeBits, mem_flags),
	};
	if ((vk_err = vkAllocateMemory(logical_gpu, &alloci, NULL, mem)))
		ERROR("\tFailed to allocate memory for buffer\n\t\"%d\"", vk_err);

	vkBindBufferMemory(logical_gpu, *buf, *mem, 0);
}

VBO _vbo_new(VkDeviceSize sz, void* verts, bool can_update, char const* file, int line, char const* fn)
{
	DEBUG(3, "[VK] Creating vertex buffer in \"%s:%d:%s\" (size: %luB)", file, line, fn, sz);
	struct Buffer tmpbuf;
	buffer_new(sz, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	              &tmpbuf.buf, &tmpbuf.mem);
	if (verts)
		buffer_update(tmpbuf, sz, verts, 0);

	VBO buf = { .sz = sz };
	buffer_new(sz, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | (can_update*VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
	           &buf.buf, &buf.mem);
	copy_buffer(buf.buf, tmpbuf.buf, sz);
	buffer_free(&tmpbuf);

	return buf;
}

IBO _ibo_new(VkDeviceSize sz, void* inds, char const* file, int line, char const* fn)
{
	DEBUG(3, "[VK] Creating index buffer in \"%s:%d:%s\" (size: %luB)", file, line, fn, sz);
	struct Buffer tmpbuf;
	buffer_new(sz, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	              &tmpbuf.buf, &tmpbuf.mem);
	buffer_update(tmpbuf, sz, inds, 0);

	IBO buf = { .sz = sz };
	buffer_new(sz, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &buf.buf, &buf.mem);
	copy_buffer(buf.buf, tmpbuf.buf, sz);
	buffer_free(&tmpbuf);

	return buf;
}

UBO _ubo_new(VkDeviceSize sz, char const* file, int line, char const* fn)
{
	DEBUG(3, "[VK] Creating uniform buffer in \"%s:%d:%s\" (size: %luB)", file, line, fn, sz);
	UBO buf = { .sz = sz };
	buffer_new(sz, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	              &buf.buf, &buf.mem);

	return buf;
}

SBO _sbo_new(VkDeviceSize sz, char const* file, int line, char const* fn)
{
	DEBUG(3, "[VK] Creating storage buffer in \"%s:%d:%s\" (size: %luB)", file, line, fn, sz);
	SBO buf = { .sz = sz };
	buffer_new(sz, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
	           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &buf.buf, &buf.mem);

	return buf;
}

void buffer_update(struct Buffer buf, VkDeviceSize sz, void* data, isize offset)
{
	void* mem;
	if ((vk_err = vkMapMemory(logical_gpu, buf.mem, offset, sz, 0, &mem)))
		ERROR("[VK] Failed to map memory\n\t\"%d\"", vk_err);
	memcpy(mem, data, sz);
	vkUnmapMemory(logical_gpu, buf.mem);
}

void _buffer_free(struct Buffer* buf, const char* file, int line, const char* fn)
{
	if (!buf->buf || !buf->mem) {
		ERROR("[VK] Buffer does not appear to be valid (probably uninitialized or double free) in \"%s:%d:%s\"",
		      file, line, fn);
		return;
	}
	vkDestroyBuffer(logical_gpu, buf->buf, NULL);
	vkFreeMemory(logical_gpu, buf->mem, NULL);
	memset(buf, 0, sizeof(struct Buffer));
}

static void copy_buffer(VkBuffer dst, VkBuffer src, VkDeviceSize sz)
{
	VkCommandBuffer buf = begin_command_buffer();
	VkBufferCopy region = {
		.srcOffset = 0,
		.dstOffset = 0,
		.size      = sz,
	};
	vkCmdCopyBuffer(buf, src, dst, 1, &region);
	end_command_buffer(buf, NULL);
}
