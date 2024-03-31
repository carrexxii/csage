#ifndef GFX_PIPELINE_H
#define GFX_PIPELINE_H

#include "vulkan/vulkan.h"

#include "util/string.h"
#include "buffers.h"
#include "image.h"

#define MAX_PIPELINES 32
#define PIPELINE_MAX_UBOS      3
#define PIPELINE_MAX_SBOS      5
#define PIPELINE_MAX_IMAGES    3
#define PIPELINE_MAX_BINDINGS  (PIPELINE_MAX_UBOS + PIPELINE_MAX_SBOS + PIPELINE_MAX_IMAGES + 1)

struct DescriptorSet {
	VkDescriptorSetLayout layout;
	VkDescriptorSet       set;
};

struct Pipeline {
	struct PipelineCreateInfo* ci;

	VkPipeline           pipeln;
	VkPipelineLayout     layout;
	struct DescriptorSet dset;

	VkShaderModule  vshader;
	VkShaderModule tcshader;
	VkShaderModule teshader;
	VkShaderModule  gshader;
	VkShaderModule  fshader;

	VkShaderStageFlags push_stages;
};

struct PipelineCreateInfo {
	int                                vert_bindc;
	VkVertexInputBindingDescription*   vert_binds;
	int                                vert_attrc;
	VkVertexInputAttributeDescription* vert_attrs;

	VkSampler           sampler;
	VkPrimitiveTopology topology;
	bool                primitive_restart;

	String  vshader;
	String  fshader;
	String tcshader;
	String teshader;
	String  gshader;

	int                push_sz;
	VkShaderStageFlags push_stages;

	int uboc;
	VkShaderStageFlags ubo_stages[PIPELINE_MAX_UBOS];
	UBO ubos[PIPELINE_MAX_UBOS];

	int sboc;
	VkShaderStageFlags sbo_stages[PIPELINE_MAX_SBOS];
	SBO sbos[PIPELINE_MAX_SBOS];

	int imgc;
	struct Image* imgs[PIPELINE_MAX_IMAGES];
};

void pipelns_init(void);
void pipelns_free(void);
struct Pipeline* pipeln_new(struct PipelineCreateInfo* ci);
void             pipeln_update(struct Pipeline* pipeln);
void             pipeln_free(struct Pipeline* pipeln);

#endif
