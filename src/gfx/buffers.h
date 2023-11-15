#ifndef GFX_BUFFER_H
#define GFX_BUFFER_H

#include <vulkan/vulkan.h>

#include "device.h"

struct Buffer {
	VkBuffer       buf;
	VkDeviceMemory mem;
	intptr         sz;
};
typedef struct Buffer VBO;
typedef struct Buffer IBO;
typedef struct Buffer UBO;
typedef struct Buffer SBO;

VkCommandBuffer begin_command_buffer();
void end_command_buffer(VkCommandBuffer buf, VkFence fence);
void buffer_new(VkDeviceSize sz, VkBufferUsageFlags use, VkMemoryPropertyFlags propfs,
                VkBuffer* buf, VkDeviceMemory* mem);
/* TODO: switch to memory size as last parameter */
VBO _vbo_new(VkDeviceSize s, void* verts, const char* file, int line, const char* fn);
IBO _ibo_new(VkDeviceSize s, void* inds, const char* file, int line, const char* fn);
UBO _ubo_new(VkDeviceSize s, const char* file, int line, const char* fn);
SBO _sbo_new(VkDeviceSize s, const char* file, int line, const char* fn);
uint find_memory_index(uint type, uint prop);

#define vbo_new(s, v) _vbo_new((s), (v), __FILE__, __LINE__, __func__)
#define ibo_new(s, i) _ibo_new((s), (i), __FILE__, __LINE__, __func__)
#define ubo_new(s)    _ubo_new((s),      __FILE__, __LINE__, __func__)
#define sbo_new(s)    _sbo_new((s),      __FILE__, __LINE__, __func__)

void buffer_update(struct Buffer buffer, VkDeviceSize sz, void* mem);

inline static void buffer_map_memory(struct Buffer buffer, VkDeviceSize sz, void** mem) {
	vkMapMemory(gpu, buffer.mem, 0, sz, 0, mem);
}
inline static void buffer_unmap_memory(struct Buffer buffer) {
	vkUnmapMemory(gpu, buffer.mem);
}

void buffer_free(struct Buffer* buf);
static inline void vbo_free(VBO* buf) { buffer_free(buf); }
static inline void ibo_free(IBO* buf) { buffer_free(buf); }
static inline void ubo_free(UBO* buf) { buffer_free(buf); }
static inline void sbo_free(SBO* buf) { buffer_free(buf); }

#endif
