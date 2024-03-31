#ifndef RESMGR_H
#define RESMGR_H

#include "vulkan/vulkan.h"

#include "util/string.h"
#include "gfx/image.h"

#define RESMGR_DEFAULT_HTABLE_SIZE 64

void resmgr_init(void);
void resmgr_printout(void);
void resmgr_free(void);

VkShaderModule load_shader(String path);
struct Image*  load_image(String path);

#endif
