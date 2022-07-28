#ifndef GFX_PIPELINE_H
#define GFX_PIPELINE_H

#include <vulkan/vulkan.h>

#include "buffers.h"  /* UBO */

struct Pipeline {
	VkPipeline            pipeln;
	VkPipelineLayout      layout;
	VkDescriptorPool      dpool;
	VkDescriptorSet       dset;
	VkDescriptorSetLayout dsetlayout;

	UBO* ubos;
	SBO  sbo;
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
	uint     uboc;
	uintptr* uboszs;
	uintptr  sbosz;
	uintptr  pushsz;
	VkShaderStageFlags pushstages;
}; static_assert(sizeof(struct Pipeline) == 168, "struct Pipeline");

void pipeln_init(struct Pipeline* pipeln, VkRenderPass renpass);
void pipeln_free(struct Pipeline* pipeln);

#endif
