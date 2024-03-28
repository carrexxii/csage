#ifndef GFX_BUFFER_H
#define GFX_BUFFER_H

#include <vulkan/vulkan.h>

#include "device.h"

struct Buffer {
	VkBuffer       buf;
	VkDeviceMemory mem;
	isize          sz;
};
typedef struct Buffer VBO;
typedef struct Buffer IBO;
typedef struct Buffer UBO;
typedef struct Buffer SBO;

VkCommandBuffer begin_command_buffer(VkQueue queue);
void end_command_buffer(VkCommandBuffer buf, VkQueue queue);
void buffer_new(VkDeviceSize sz, VkBufferUsageFlags buf_flags, VkMemoryPropertyFlags mem_flags, VkBuffer* buf, VkDeviceMemory* mem);
// TODO: switch to memory size as last parameter
VBO _vbo_new(VkDeviceSize s, void* verts, bool can_update, const char* file, int line, const char* fn);
IBO _ibo_new(VkDeviceSize s, void* inds, const char* file, int line, const char* fn);
UBO _ubo_new(VkDeviceSize s, const char* file, int line, const char* fn);
SBO _sbo_new(VkDeviceSize s, const char* file, int line, const char* fn);

#define vbo_new(s, v, u) _vbo_new((s), (v), (u), __FILE__, __LINE__, __func__)
#define ibo_new(s, i)    _ibo_new((s), (i),      __FILE__, __LINE__, __func__)
#define ubo_new(s)       _ubo_new((s),           __FILE__, __LINE__, __func__)
#define sbo_new(s)       _sbo_new((s),           __FILE__, __LINE__, __func__)

void buffer_update(struct Buffer buf, VkDeviceSize sz, void* mem, isize offset);

inline static void buffer_map_memory(struct Buffer buf, VkDeviceSize sz, void** mem) {
	vkMapMemory(logical_gpu, buf.mem, 0, sz, 0, mem);
}
inline static void buffer_unmap_memory(struct Buffer buf) {
	vkUnmapMemory(logical_gpu, buf.mem);
}

void _buffer_free(struct Buffer* buf, const char* file, int line, const char* fn);
#define buffer_free(buf) _buffer_free(buf, __FILE__, __LINE__, __func__)
#define vbo_free(buf) _Generic(buf, VBO*: buffer_free(buf))
#define ibo_free(buf) _Generic(buf, IBO*: buffer_free(buf))
#define ubo_free(buf) _Generic(buf, UBO*: buffer_free(buf))
#define sbo_free(buf) _Generic(buf, SBO*: buffer_free(buf))

#endif
