#ifndef MATHS_SCRATCH_H
#define MATHS_SCRATCH_H

#include <vulkan/vulkan.h>

#include "pga.h"
#include "camera.h"

#define SCRATCH_DEFAULT_ELEMENT_COUNT 8

void scratch_init(VkRenderPass renderpass);
void scratch_load(void);
void scratch_record_commands(VkCommandBuffer cmd_buf, struct Camera* cam);
void scratch_clear(void);
void scratch_free(void);

#define scratch_add(a) _Generic(a, \
		Vec   : scratch_add_vec,   \
		Bivec : scratch_add_bivec, \
		Trivec: scratch_add_trivec \
	)(a)
void scratch_add_vec(Vec a);
void scratch_add_bivec(Bivec a);
void scratch_add_trivec(Trivec a);

#endif
