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
		.commandPool        = cmdpool,
		.commandBufferCount = 1,
	};
	vkAllocateCommandBuffers(gpu, &bufi, &buf);

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
	vkFreeCommandBuffers(gpu, cmdpool, 1, &buf);
}

void buffer_new(VkDeviceSize sz, VkBufferUsageFlags usefs, VkMemoryPropertyFlags propfs, VkBuffer* buf,
                   VkDeviceMemory* mem)
{
	VkBufferCreateInfo bufi = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size                  = sz,
		.usage                 = usefs,
		.flags                 = 0,
		.sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices   = NULL,
	};
	if (vkCreateBuffer(gpu, &bufi, alloccb, buf) != VK_SUCCESS)
		ERROR("\tFailed to create buffer");

	VkMemoryRequirements memreq;
	vkGetBufferMemoryRequirements(gpu, *buf, &memreq);
	VkMemoryAllocateInfo alloci = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize  = memreq.size,
		.memoryTypeIndex = find_memory_index(memreq.memoryTypeBits, propfs),
	};
	if (vkAllocateMemory(gpu, &alloci, alloccb, mem) != VK_SUCCESS)
		ERROR("\tFailed to allocate memory for buffer");

	vkBindBufferMemory(gpu, *buf, *mem, 0);
}

VBO _vbo_new(VkDeviceSize sz, void* verts, char const* file, int line, char const* fn)
{
	DEBUG(3, "[VK] Creating vertex buffer in \"%s:%d:%s\" (size: %luB)", file, line, fn, sz);
	struct Buffer tmpbuf;
	buffer_new(sz, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
	                                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	              &tmpbuf.buf, &tmpbuf.mem);
	buffer_update(tmpbuf, sz, verts);

	VBO buf = { .sz = sz };
	buffer_new(sz, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &buf.buf, &buf.mem);
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
	buffer_update(tmpbuf, sz, inds);

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

void buffer_update(struct Buffer buf, VkDeviceSize sz, void* data)
{
	void* mem;
	vkMapMemory(gpu, buf.mem, 0, sz, 0, &mem);
	memcpy(mem, data, sz);
	vkUnmapMemory(gpu, buf.mem);
}

void buffer_free(struct Buffer* buf)
{
	vkDestroyBuffer(gpu, buf->buf, alloccb);
	vkFreeMemory(gpu, buf->mem, alloccb);
	buf->sz = 0;
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

uint find_memory_index(uint type, uint prop)
{
	VkPhysicalDeviceMemoryProperties memprop;
	vkGetPhysicalDeviceMemoryProperties(physicalgpu, &memprop);
	for (uint i = 0; i < memprop.memoryTypeCount; i++)
		if (type & (1 << i) && (memprop.memoryTypes[i].propertyFlags & prop) == prop)
			return i;

	ERROR("[VK] Failed to find suitable memory type");
	return UINT_MAX;
}
