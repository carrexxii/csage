#ifndef GFX_PIPELINE_H
#define GFX_PIPELINE_H

#include <vulkan/vulkan.h>

#include "buffers.h"  /* UBO */

struct Pipeline {
	VkPipeline            pipeln;
	VkPipelineLayout      lay;
	VkDescriptorPool      dpool;
	VkDescriptorSet       dset;
	VkDescriptorSetLayout dsetlay;

	UBO ubo;
	/* Caller-defined values */
	struct {
		VkVertexInputBindingDescription*   vbinds;
		VkVertexInputAttributeDescription* vattrs;
		uint vbindc;
		uint vattrc;
	};
	VkShaderModule  vshader;
	VkShaderModule tcshader;
	VkShaderModule teshader;
	VkShaderModule  gshader;
	VkShaderModule  fshader;
	uint ubosz;
	uint pushsz;
	VkShaderStageFlags pushstages;
}; static_assert(sizeof(struct Pipeline) == 136, "struct Pipeline");

void pipeln_init(struct Pipeline* pipeln, VkRenderPass renpass);
void pipeln_free(struct Pipeline* pipeln);

#endif
