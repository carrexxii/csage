#ifndef GFX_PIPELINE_H
#define GFX_PIPELINE_H

#include <vulkan/vulkan.h>

#include "buffers.h"

/* !! Uniform buffers must be aligned as follows:
 * - float            -> alignment of 4
 * - vec2             -> alignment of 8
 * - vec3, vec4, mat4 -> alignment of 16
 */

struct Pipeline {
	VkPipeline            pipeln;
	VkPipelineLayout      layout;
	VkDescriptorPool      dpool;
	VkDescriptorSet       dset;
	VkDescriptorSetLayout dsetlayout;

	/* Caller-defined values */
	VkPrimitiveTopology topology;
	struct {
		int vertbindc;
		int vertattrc;
		VkVertexInputBindingDescription*   vertbinds;
		VkVertexInputAttributeDescription* vertattrs;
	};
	VkShaderModule  vshader;
	VkShaderModule tcshader;
	VkShaderModule teshader;
	VkShaderModule  gshader;
	VkShaderModule  fshader;
	intptr   uboc;
	UBO*     ubos;
	intptr   sbosz;
	SBO*     sbo;
	intptr   pushsz;
	struct Texture* textures;
	intptr texturec;
	VkShaderStageFlags pushstages;
};

void init_pipeln(struct Pipeline* pipeln, VkRenderPass renpass);
void pipeln_free(struct Pipeline* pipeln);

#endif
