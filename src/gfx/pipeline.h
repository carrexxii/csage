#ifndef GFX_PIPELINE_H
#define GFX_PIPELINE_H

#include <vulkan/vulkan.h>

#include "buffers.h"

#define PIPELINE_MAX_BINDINGS        16
#define PIPELINE_MAX_DESCRIPTOR_SETS 64
#define PIPELINE_MAX_IMAGES_PER_DSET 8

// TODO: maybe make the descriptor layout parameters
/* Pipeline layout:
 * - If you have images, call `pipeln_alloc_dsets()` first (`pipeln_init()` will
 *   automatically call it if it has not already been called)
 * - Use `pipeln_create_image_dset` to create descriptors for images before calling `pipeln_init`
 * - Shader descriptor layout is: [Set 0 => [sampler][storage buffers][uniform buffers]]
 *                                [Set 1 => [images]]
 * - The pipeline will only have the descriptor for sampler/storage/uniform buffers, but will
 *   allocate for `dset_cap` descriptors
 * - No values must be changed once `pipeln_init` has been called
 */
struct Pipeline {
	VkPipeline            pipeln;
	VkPipelineLayout      layout;
	VkDescriptorPool      dpool;
	VkDescriptorSet*      dset;   /* This is just a pointer to dsets[0] */
	VkDescriptorSet*      dsets;
	VkDescriptorSetLayout dset_data_layout;
	VkDescriptorSetLayout dset_img_layout;
	isize dsetc;

	/*** Caller-defined values ***/
	VkSampler           sampler;  /* NULL = use `default_sampler`                     */
	VkPrimitiveTopology topology; /* 0    = use `VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST` */

	int vert_bindc;
	int vert_attrc;
	VkVertexInputBindingDescription*   vert_binds;
	VkVertexInputAttributeDescription* vert_attrs;

	VkShaderModule  vshader;
	VkShaderModule tcshader;
	VkShaderModule teshader;
	VkShaderModule  gshader;
	VkShaderModule  fshader;

	isize push_sz;
	VkShaderStageFlags push_stages;

	isize  uboc;
	UBO*   ubos;
	isize  sboc;
	isize* sbo_szs;
	SBO*   sbos;

	isize dset_cap; /* How many sets of images             */
	isize imgc;     /* Number of images used in the shader */
};

void pipeln_alloc_dsets(struct Pipeline* pipeln);
void pipeln_init(struct Pipeline* pipeln, VkRenderPass renpass);
VkDescriptorSet pipeln_create_image_dset(struct Pipeline* pipeln, int img_viewc, VkImageView* img_views);
void pipeln_free(struct Pipeline* pipeln);

#endif
