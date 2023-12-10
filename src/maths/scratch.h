#ifndef MATHS_SCRATCH_H
#define MATHS_SCRATCH_H

#include <vulkan/vulkan.h>

void scratch_init(VkRenderPass renderpass);
void scratch_record_commands(VkCommandBuffer cmd_buf);
void scratch_free(void);

#endif
