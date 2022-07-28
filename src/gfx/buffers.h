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
void create_buffer(VkDeviceSize sz, VkBufferUsageFlags use, VkMemoryPropertyFlags propfs,
                   VkBuffer* buf, VkDeviceMemory* mem);
/* TODO: switch to memory size as last parameter */
VBO _create_vbo(VkDeviceSize s, void* verts, const char* file, int line, const char* fn);
IBO _create_ibo(VkDeviceSize s, void* inds, const char* file, int line, const char* fn);
UBO _create_ubo(VkDeviceSize s, const char* file, int line, const char* fn);
SBO _create_sbo(VkDeviceSize s, const char* file, int line, const char* fn);
uint find_memory_index(uint type, uint prop);

#define create_vbo(s, v) _create_vbo((s), (v), __FILE__, __LINE__, __func__)
#define create_ibo(s, i) _create_ibo((s), (i), __FILE__, __LINE__, __func__)
#define create_ubo(s)    _create_ubo((s),      __FILE__, __LINE__, __func__)
#define create_sbo(s)    _create_sbo((s),      __FILE__, __LINE__, __func__)

void update_buffer(struct Buffer buf, VkDeviceSize sz, void* data);
static inline void update_vbo(VBO buf, VkDeviceSize sz, void* data) { update_buffer(buf, sz, data); }
static inline void update_ibo(IBO buf, VkDeviceSize sz, void* data) { update_buffer(buf, sz, data); }
static inline void update_ubo(UBO buf, VkDeviceSize sz, void* data) { update_buffer(buf, sz, data); }
static inline void update_sbo(SBO buf, VkDeviceSize sz, void* data) { update_buffer(buf, sz, data); }

void free_buffer(struct Buffer buf);
static inline void free_vbo(VBO buf) { free_buffer(buf); }
static inline void free_ibo(IBO buf) { free_buffer(buf); }
static inline void free_ubo(UBO buf) { free_buffer(buf); }
static inline void free_sbo(SBO buf) { free_buffer(buf); }

#endif
