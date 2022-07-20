#ifndef GFX_BUFFER_H
#define GFX_BUFFER_H

#include <vulkan/vulkan.h>

struct Buffer {
	VkBuffer buf;
	VkDeviceMemory mem;
}; static_assert(sizeof(struct Buffer) == 16, "struct Buffer");
typedef struct Buffer VBO;
typedef struct Buffer IBO;
typedef struct Buffer UBO;
typedef struct Buffer SBO;

VkCommandBuffer begin_command_buffer();
void end_command_buffer(VkCommandBuffer buf, VkFence fence);
void buffer_new(VkDeviceSize s, VkBufferUsageFlags use,
	VkMemoryPropertyFlags prop, VkBuffer* buf, VkDeviceMemory* mem);
/* TODO: switch to memory size as last parameter */
VBO _vbo_new(VkDeviceSize s, void* verts, const char* file, int line, const char* fn);
IBO _ibo_new(VkDeviceSize s, void* inds, const char* file, int line, const char* fn);
UBO _ubo_new(VkDeviceSize s, const char* file, int line, const char* fn);
SBO _sbo_new(VkDeviceSize s, void* mem, const char* file, int line, const char* fn);
uint find_mem_index(uint type, uint prop);

#define vbo_new(s, v) _vbo_new((s), (v), __FILE__, __LINE__, __func__)
#define ibo_new(s, i) _ibo_new((s), (i), __FILE__, __LINE__, __func__)
#define ubo_new(s)    _ubo_new((s),      __FILE__, __LINE__, __func__)
#define sbo_new(s, m) _sbo_new((s), (m), __FILE__, __LINE__, __func__)

void buf_free(struct Buffer buf);
static inline void vbo_free(VBO buf) { buf_free(buf); }
static inline void ibo_free(IBO buf) { buf_free(buf); }
static inline void ubo_free(UBO buf) { buf_free(buf); }
static inline void sbo_free(SBO buf) { buf_free(buf); }

#endif
