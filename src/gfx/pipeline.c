#include <vulkan/vulkan.h>

#include "config.h"
#include "vulkan.h"
#include "device.h"
#include "buffers.h"
#include "image.h"
#include "texture.h"
#include "swapchain.h"
#include "renderer.h"
#include "pipeline.h"

static int init_shaders(struct Pipeline* pipeln, VkPipelineShaderStageCreateInfo* stagecis);
inline static VkDescriptorSet pipeln_get_dset(struct Pipeline* pipeln);
inline static VkDescriptorSetLayoutBinding create_dset_layout(VkShaderStageFlagBits stagef, VkDescriptorType type, int binding);
inline static void bind_sampler(VkDescriptorSet dset, int binding, VkSampler samp);
inline static void bind_buffer(VkDescriptorSet dset, int binding, VkDescriptorType type, VkBuffer buf, isize offset, isize range);

void pipeln_alloc_dsets(struct Pipeline* pipeln)
{
	/* Data bindings (set 0) */
	int dset_bindingc = 1;
	VkDescriptorSetLayoutBinding dset_bindings[PIPELINE_MAX_BINDINGS] = {
		[0] = create_dset_layout(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLER, 0),
	};
	for (int i = 0; i < pipeln->sboc; i++) {
		dset_bindings[dset_bindingc] = create_dset_layout(VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, dset_bindingc);
		dset_bindingc++;
	}
	for (int i = 0; i < pipeln->uboc; i++) {
		dset_bindings[dset_bindingc] = create_dset_layout(VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, dset_bindingc);
		dset_bindingc++;
	}
	VkDescriptorSetLayoutCreateInfo dset_bindingsi = {
		.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = dset_bindingc,
		.pBindings    = dset_bindings,
	};
	if ((vk_err = vkCreateDescriptorSetLayout(logical_gpu, &dset_bindingsi, NULL, &pipeln->dset_data_layout)))
		ERROR("[VK] Failed to create descriptor set layout (%d)", vk_err);
	else
		DEBUG(3, "[VK] Created descriptor set layout with %d descriptors", dset_bindingc);

	/* Image bindings (sets 1+) */
	dset_bindingc = 0;
	for (int i = 0; i < pipeln->imgc; i++) {
		dset_bindings[dset_bindingc] = create_dset_layout(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, dset_bindingc);
		dset_bindingc++;
	}
	dset_bindingsi = (VkDescriptorSetLayoutCreateInfo){
		.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = dset_bindingc,
		.pBindings    = dset_bindings,
	};
	if ((vk_err = vkCreateDescriptorSetLayout(logical_gpu, &dset_bindingsi, NULL, &pipeln->dset_img_layout)))
		ERROR("[VK] Failed to create descriptor set layout (%d)", vk_err);
	else
		DEBUG(3, "[VK] Created descriptor set layout with %d descriptors", dset_bindingc);

	int pool_szc = 1;
	VkDescriptorPoolSize pool_szs[4] = {
		[0] = (VkDescriptorPoolSize){ .type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = 2, }, // !!
	};
	if (pipeln->sboc)
		pool_szs[pool_szc++] = (VkDescriptorPoolSize){
			.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = pipeln->sboc,
		};
	if (pipeln->uboc)
		pool_szs[pool_szc++] = (VkDescriptorPoolSize){
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = pipeln->uboc,
		};
	if (pipeln->imgc)
		pool_szs[pool_szc++] = (VkDescriptorPoolSize){
			.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = pipeln->imgc * pipeln->dset_cap + 1, // !!
		};
	VkDescriptorPoolCreateInfo dpoolci = {
		.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.poolSizeCount = pool_szc,
		.pPoolSizes    = pool_szs,
		.maxSets       = ++pipeln->dset_cap, // TODO: Check this
	};
	if ((vk_err = vkCreateDescriptorPool(logical_gpu, &dpoolci, NULL, &pipeln->dpool)))
		ERROR("[VK] Failed to create descriptor pool (%d)", vk_err);
	else
		DEBUG(3, "[VK] Created descriptor pool with %d descriptor sets (%ld slots for images)", pool_szc, pipeln->imgc);

	VkDescriptorSetLayout layouts[PIPELINE_MAX_DESCRIPTOR_SETS];
	layouts[0] = pipeln->dset_data_layout;
	for (int i = 1; i < pipeln->dset_cap; i++)
		layouts[i] = pipeln->dset_img_layout;
	pipeln->dsets = smalloc(pipeln->dset_cap*sizeof(VkDescriptorSet));
	pipeln->dset  = &pipeln->dsets[0];
	pipeln->dsetc = 1;
	VkDescriptorSetAllocateInfo dsetalloci = {
		.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool     = pipeln->dpool,
		.descriptorSetCount = pipeln->dset_cap,
		.pSetLayouts        = layouts,
	};
	if ((vk_err = vkAllocateDescriptorSets(logical_gpu, &dsetalloci, pipeln->dsets)))
		ERROR("[VK] Failed to allocate for %d descriptor sets\n\t\"%d\"", dsetalloci.descriptorSetCount, vk_err); // TODO: STRING_OF_VK_ERR
	else
		DEBUG(3, "[VK] Allocated %ld descriptor sets", pipeln->dset_cap);
}

void pipeln_init(struct Pipeline* pipeln, VkRenderPass renderpass)
{
	VkPipelineShaderStageCreateInfo stagesci[5];
	int stagec = init_shaders(pipeln, stagesci);

	VkPipelineVertexInputStateCreateInfo vinci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pVertexBindingDescriptions      = pipeln->vert_binds,
		.vertexBindingDescriptionCount   = pipeln->vert_bindc,
		.pVertexAttributeDescriptions    = pipeln->vert_attrs,
		.vertexAttributeDescriptionCount = pipeln->vert_attrc,
	};
	VkPipelineInputAssemblyStateCreateInfo inassci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = pipeln->topology? pipeln->topology: VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = pipeln->topology && pipeln->topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST? true: false,
	};
	VkViewport viewport = {
		.x        = 0.0,
		.y        = 0.0,
		.width    = (float)swapchain.ext.width,
		.height   = (float)swapchain.ext.height,
		.minDepth = 0.0,
		.maxDepth = 1.0,
	};
	VkRect2D scissor = {
		.offset = { 0, 0 },
		.extent = swapchain.ext,
	};
	VkPipelineViewportStateCreateInfo viewportsci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports    = &viewport,
		.scissorCount  = 1,
		.pScissors     = &scissor,
	};
	VkPipelineRasterizationStateCreateInfo rasterci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.rasterizerDiscardEnable = false,
		.polygonMode             = VK_POLYGON_MODE_FILL,
		.lineWidth               = 1.0,
		.cullMode                = VK_CULL_MODE_BACK_BIT,
		.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthClampEnable        = false,
		.depthBiasEnable         = false,
		.depthBiasClamp          = 0.0,
		.depthBiasConstantFactor = 0.0,
		.depthBiasSlopeFactor    = 0.0,
	};
	VkPipelineMultisampleStateCreateInfo msci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.sampleShadingEnable   = false,
		.rasterizationSamples  = gpu_properties.max_samples,
		.minSampleShading      = 1.0,
		.pSampleMask           = NULL,
		.alphaToCoverageEnable = false,
		.alphaToOneEnable      = false,
	};
	VkPipelineColorBlendAttachmentState blendattachs = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
						  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = true,

		.colorBlendOp        = VK_BLEND_OP_ADD,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,

		.alphaBlendOp        = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
	};
	VkPipelineColorBlendStateCreateInfo blendsci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOp         = VK_LOGIC_OP_COPY,
		.logicOpEnable   = false,
		.attachmentCount = 1,
		.pAttachments    = &blendattachs,
		.blendConstants  = { 0.0, 0.0, 0.0, 0.0 },
	};
	VkPipelineDepthStencilStateCreateInfo stencilsci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable       = true,
		.depthWriteEnable      = true,
		.depthCompareOp        = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = false,
		.minDepthBounds        = 0.0,
		.maxDepthBounds        = 1.0,
		.stencilTestEnable     = false,
		.front                 = { 0 },
		.back                  = { 0 },
	};
	VkPushConstantRange tpushr = {
		.stageFlags = pipeln->push_stages,
		.offset     = 0,
		.size       = pipeln->push_sz,
	};
	VkPipelineLayoutCreateInfo tlaysci = {
		.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount         = 2,
		.pSetLayouts            = (VkDescriptorSetLayout[]){ pipeln->dset_data_layout, pipeln->dset_img_layout },
		.pushConstantRangeCount = pipeln->push_sz > 0? 1: 0,
		.pPushConstantRanges    = &tpushr,
	};
	if (vkCreatePipelineLayout(logical_gpu, &tlaysci, NULL, &pipeln->layout))
		ERROR("[VK] Failed to create pipeline layout");
	else
		DEBUG(4, "[VK] Created pipeline layout");

	VkGraphicsPipelineCreateInfo tpipelnci = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount          = stagec,
		.pStages             = stagesci,
		.pVertexInputState   = &vinci,
		.pInputAssemblyState = &inassci,
		.pViewportState      = &viewportsci,
		.pRasterizationState = &rasterci,
		.pMultisampleState   = &msci,
		.pDepthStencilState  = &stencilsci,
		.pColorBlendState    = &blendsci,
		.pDynamicState       = NULL,
		.layout              = pipeln->layout,
		.renderPass          = renderpass,
		.subpass             = 0,
		.basePipelineHandle  = NULL,
		.basePipelineIndex   = -1,
	};
	if (vkCreateGraphicsPipelines(logical_gpu, NULL, 1, &tpipelnci, NULL, &pipeln->pipeln))
		ERROR("[VK] Failed to create pipeline");
	else
		DEBUG(3, "[VK] Pipeline created");
}

VkDescriptorSet pipeln_create_image_dset(struct Pipeline* pipeln, VkImageView* img_views)
{
	VkDescriptorSet      dset = pipeln_get_dset(pipeln);
	VkWriteDescriptorSet dset_writes[PIPELINE_MAX_IMAGES_PER_DSET];
	for (int i = 0; i < pipeln->imgc; i++) {
		dset_writes[i] = (VkWriteDescriptorSet) {
			.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet           = dset,
			.dstBinding       = i,
			.dstArrayElement  = 0,
			.descriptorCount  = 1,
			.descriptorType   = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.pBufferInfo      = NULL,
			.pTexelBufferView = NULL,
			.pImageInfo       = &(VkDescriptorImageInfo){
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.imageView   = img_views[i],
				.sampler     = NULL,
			},
		};
	}

	vkUpdateDescriptorSets(logical_gpu, pipeln->imgc, dset_writes, 0, NULL);
	return dset;
}

void pipeln_free(struct Pipeline* pipeln)
{
	DEBUG(3, "[VK] Destroying pipeline (%p)...", (void*)pipeln);
	vkDestroyPipeline(logical_gpu, pipeln->pipeln, NULL);
	vkDestroyPipelineLayout(logical_gpu, pipeln->layout, NULL);
	vkDestroyDescriptorPool(logical_gpu, pipeln->dpool, NULL);

	if (pipeln->vshader)  vkDestroyShaderModule(logical_gpu, pipeln->vshader , NULL);
	if (pipeln->tcshader) vkDestroyShaderModule(logical_gpu, pipeln->tcshader, NULL);
	if (pipeln->teshader) vkDestroyShaderModule(logical_gpu, pipeln->teshader, NULL);
	if (pipeln->gshader)  vkDestroyShaderModule(logical_gpu, pipeln->gshader , NULL);
	if (pipeln->fshader)  vkDestroyShaderModule(logical_gpu, pipeln->fshader , NULL);

	vkDestroyDescriptorSetLayout(logical_gpu, pipeln->dset_data_layout, NULL);
	vkDestroyDescriptorSetLayout(logical_gpu, pipeln->dset_img_layout , NULL);
}

/* -------------------------------------------------------------------- */

static int init_shaders(struct Pipeline* pipeln, VkPipelineShaderStageCreateInfo* stagecis)
{
	int stageic = 0;
	if (pipeln->vshader)
		stagecis[stageic++] = (VkPipelineShaderStageCreateInfo){
			.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage  = VK_SHADER_STAGE_VERTEX_BIT,
			.module = pipeln->vshader,
			.pName  = "main",
		};
	if (pipeln->gshader)
		stagecis[stageic++] = (VkPipelineShaderStageCreateInfo){
			.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage  = VK_SHADER_STAGE_GEOMETRY_BIT,
			.module = pipeln->gshader,
			.pName  = "main",
		};
	if (pipeln->fshader)
		stagecis[stageic++] = (VkPipelineShaderStageCreateInfo){
			.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = pipeln->fshader,
			.pName  = "main",
		};

	if (!pipeln->dsets)
		pipeln_alloc_dsets(pipeln);

	int total = 1, sboc, uboc;
	bind_sampler(*pipeln->dset, 0, pipeln->sampler? pipeln->sampler: default_sampler);
	for (sboc = 0; sboc < pipeln->sboc; total++, sboc++)
		bind_buffer(*pipeln->dset, total, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, pipeln->sbos[sboc].buf, 0, pipeln->sbo_szs[sboc]);
	for (uboc = 0; uboc < pipeln->uboc; total++, uboc++)
		bind_buffer(*pipeln->dset, total, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, pipeln->ubos[uboc].buf, 0, pipeln->ubos[uboc].sz);

	DEBUG(4, "[GFX] Initialized %d shaders with %d buffers bound (SBOs: %d; UBOs: %d)", stageic, total, sboc, uboc);
	return stageic;
}

inline static VkDescriptorSet pipeln_get_dset(struct Pipeline* pipeln)
{
	if (pipeln->dsetc > pipeln->dset_cap) {
		ERROR("[GFX] No more descriptor sets remain in the pipeline's pool (cap: %ld)", pipeln->dset_cap);
		return NULL;
	}

	return pipeln->dsets[pipeln->dsetc++];
}

inline static VkDescriptorSetLayoutBinding create_dset_layout(VkShaderStageFlagBits stagef, VkDescriptorType type, int binding)
{
	return (VkDescriptorSetLayoutBinding){
		.binding            = binding,
		.stageFlags         = stagef,
		.descriptorCount    = 1,
		.descriptorType     = type,
		.pImmutableSamplers = NULL,
	};
}

inline static void bind_sampler(VkDescriptorSet dset, int binding, VkSampler samp)
{
	VkWriteDescriptorSet dset_write = {
		.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet           = dset,
		.dstBinding       = binding,
		.dstArrayElement  = 0,
		.descriptorCount  = 1,
		.descriptorType   = VK_DESCRIPTOR_TYPE_SAMPLER,
		.pTexelBufferView = NULL,
		.pBufferInfo      = NULL,
		.pImageInfo       = &(VkDescriptorImageInfo){
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.imageView   = NULL,
			.sampler     = samp,
		},
	};

	vkUpdateDescriptorSets(logical_gpu, 1, &dset_write, 0, NULL);
}

inline static void bind_buffer(VkDescriptorSet dset, int binding, VkDescriptorType type, VkBuffer buf, isize offset, isize range)
{
	VkWriteDescriptorSet dset_write = {
		.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet           = dset,
		.dstBinding       = binding,
		.dstArrayElement  = 0,
		.descriptorCount  = 1,
		.descriptorType   = type,
		.pTexelBufferView = NULL,
		.pImageInfo       = NULL,
		.pBufferInfo      = &(VkDescriptorBufferInfo){
			.buffer = buf,
			.offset = offset,
			.range  = range,
		},
	};

	vkUpdateDescriptorSets(logical_gpu, 1, &dset_write, 0, NULL);
}
