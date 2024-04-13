#ifndef RESMGR_H
#define RESMGR_H

#include <vulkan/vulkan.h>

#include "common.h"
#include "gfx/image.h"

#define RESMGR_DEFAULT_HTABLE_SIZE 64

#define STRING_OF_RESOURCE_TYPE(x) ((x) < RES_MAX? string_of_handle_type[x]: "<Not a handle type>")
typedef enum ResourceType {
	RES_NONE,
	RES_FILE,
	RES_SHADER,
	RES_IMAGE,
	RES_BUFFER,
	RES_PIPELINE,
	RES_MAX,
} ResourceType;
extern const char* string_of_handle_type[];

void resmgr_init(void);
void resmgr_printout(void);
void resmgr_defer(ResourceType type, void* data);
void resmgr_clean(void);
void resmgr_free(void);

VkShaderModule load_shader(String path);
struct Image*  load_image(String path);

void enumerate_dir(const char* path, bool with_path, bool with_ext, String ext, struct VArray* arr, struct Arena* arena);

#endif

