#ifndef RESMGR_H
#define RESMGR_H

#include "vulkan/vulkan.h"

#include "util/string.h"
#include "gfx/image.h"

#define RESMGR_DEFAULT_HTABLE_SIZE 64

#define STRING_OF_RESOURCE_TYPE(x) ((x) < RES_MAX? string_of_handle_type[x]: "<Not a handle type>")
enum ResourceType {
	RES_NONE,
	RES_FILE,
	RES_SHADER,
	RES_IMAGE,
	RES_UBO,
	RES_SBO,
	RES_PIPELINE,
	RES_MAX,
};
extern const char* string_of_handle_type[];

void resmgr_init(void);
void resmgr_printout(void);
void resmgr_defer(enum ResourceType type, void* data);
void resmgr_clean(void);
void resmgr_free(void);

VkShaderModule load_shader(String path);
struct Image*  load_image(String path);

#endif
