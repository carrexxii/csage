#ifndef MATHS_SCRATCH_H
#define MATHS_SCRATCH_H

#include "vulkan/vulkan.h"

#include "types.h"
#include "camera.h"

#define SCRATCH_DEFAULT_ELEMENT_COUNT 8
#define SCRATCH_POINT_COLOUR          COLOUR_GREEN
#define SCRATCH_LINE_COLOUR           COLOUR_BLUE
#define SCRATCH_PLANE_COLOUR          COLOUR_CYAN
#define SCRATCH_OPACITY               0.3f
#define SCRATCH_AXIS_OPACITY          0.7f

void scratch_init(void);
void scratch_load(void);
void scratch_clear(void);
void scratch_record_commands(VkCommandBuffer cmd_buf, struct Camera* cam);
void scratch_free(void);

#define scratch_add(a) _Generic(a, \
		Vec    : scratch_add_vec,   \
		Bivec  : scratch_add_bivec,  \
		Trivec : scratch_add_trivec,  \
		Trivec*: scratch_add_trivecs   \
	)(a)
void scratch_add_vec(Vec a);
void scratch_add_bivec(Bivec a);
void scratch_add_trivec(Trivec a);
void scratch_add_trivecs(Trivec a[4]);

#endif
