#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

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
	if (!pipeln->dset_cap)
		return;

	// TODO: vkResetDescriptorPool
	int pool_szc = 0;
	VkDescriptorPoolSize pool_szs[4];
	if (pipeln->sboc)
		pool_szs[pool_szc++] = (VkDescriptorPoolSize){
			.type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = pipeln->sboc * pipeln->dset_cap,
		};
	if (pipeln->uboc)
		pool_szs[pool_szc++] = (VkDescriptorPoolSize){
			.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = pipeln->uboc * pipeln->dset_cap,
		};
	if (pipeln->imgc) {
		pool_szs[pool_szc++] = (VkDescriptorPoolSize){
			.type            = VK_DESCRIPTOR_TYPE_SAMPLER,
			.descriptorCount = pipeln->dset_cap,
		};
		pool_szs[pool_szc++] = (VkDescriptorPoolSize){
			.type            = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = pipeln->imgc * pipeln->dset_cap,
		};
	}
	// https://www.reddit.com/r/vulkan/comments/8u9zqr/comment/e1e8d5f/
	VkDescriptorPoolCreateInfo dpoolci = {
		.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.poolSizeCount = pool_szc,
		.pPoolSizes    = pool_szs,
		.maxSets       = pipeln->dset_cap,
	};
	if ((vk_err = vkCreateDescriptorPool(logical_gpu, &dpoolci, NULL, &pipeln->dpool)))
		ERROR("[VK] Failed to create descriptor pool\n\t\"%d\"", vk_err);
	else
		DEBUG(3, "[VK] Created descriptor pool with %d different descriptors in %lu sets", pool_szc, pipeln->dset_cap);

	pipeln->dsets = smalloc(pipeln->dset_cap*sizeof(VkDescriptorSet));
}

void pipeln_init(struct Pipeline* pipeln)
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
		.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology               = pipeln->topology,
		.primitiveRestartEnable = pipeln->topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST &&
		                          pipeln->topology != VK_PRIMITIVE_TOPOLOGY_LINE_LIST? true: false,
	};
	VkViewport viewport = {
		.x        = 0.0f,
		.y        = 0.0f,
		.width    = (float)swapchain.ext.width,
		.height   = (float)swapchain.ext.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	VkRect2D scissor = {
		.offset = { 0, 0 },
		.extent = swapchain.ext,
	};
	VkPipelineViewportStateCreateInfo viewportsci = {
		.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports    = &viewport,
		.scissorCount  = 1,
		.pScissors     = &scissor,
	};
	VkPipelineRasterizationStateCreateInfo rasterci = {
		.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.rasterizerDiscardEnable = false,
		.polygonMode             = VK_POLYGON_MODE_FILL,
		.lineWidth               = 3.0f,
		.cullMode                = VK_CULL_MODE_BACK_BIT,
		.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthClampEnable        = false,
		.depthBiasEnable         = false,
		.depthBiasClamp          = 0.0f,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasSlopeFactor    = 0.0f,
	};
	VkPipelineMultisampleStateCreateInfo msci = {
		.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.sampleShadingEnable   = false,
		.rasterizationSamples  = gpu_properties.max_samples,
		.minSampleShading      = 1.0f,
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
		.setLayoutCount         = !!pipeln->dset_cap,
		.pSetLayouts            = &pipeln->dset_layout,
		.pushConstantRangeCount = !!pipeln->push_sz,
		.pPushConstantRanges    = &tpushr,
	};
	if ((vk_err = vkCreatePipelineLayout(logical_gpu, &tlaysci, NULL, &pipeln->layout)))
		ERROR("[VK] Failed to create pipeline layout\n\t\"%d\"", vk_err);
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
	if ((vk_err = vkCreateGraphicsPipelines(logical_gpu, NULL, 1, &tpipelnci, NULL, &pipeln->pipeln)))
		ERROR("[VK] Failed to create pipeline\n\t\"%d\"", vk_err);
	else
		DEBUG(3, "[VK] Pipeline created");
}

VkDescriptorSet pipeln_create_dset(struct Pipeline* pipeln, int uboc, UBO* ubos, int sboc, SBO* sbos, int img_viewc, VkImageView* img_views)
{
	assert(uboc <= 10 && sboc <= 10);

	if (!pipeln->dset_layout) {
		VkDescriptorSetLayoutBinding dset_bindings[PIPELINE_MAX_BINDINGS];
		int dset_bindingc = 0;
		int binding       = 0;
		for (int i = 0; i < uboc; i++)
			dset_bindings[dset_bindingc++] = create_dset_layout(VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding++);
		binding = 10;
		for (int i = 0; i < sboc; i++)
			dset_bindings[dset_bindingc++] = create_dset_layout(VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, binding++);
		binding = 20;
		if (img_viewc) {
			dset_bindings[dset_bindingc++] = create_dset_layout(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLER, binding++);
			for (int i = 0; i < img_viewc; i++)
				dset_bindings[dset_bindingc++] = create_dset_layout(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, binding++);
		}
		VkDescriptorSetLayoutCreateInfo dset_bindingsi = {
			.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = dset_bindingc,
			.pBindings    = dset_bindings,
		};
		if ((vk_err = vkCreateDescriptorSetLayout(logical_gpu, &dset_bindingsi, NULL, &pipeln->dset_layout)))
			ERROR("[VK] Failed to create descriptor set layout (%d)", vk_err);
		else
			DEBUG(3, "[VK] Created descriptor set layout with %d descriptors", dset_bindingc);
	}

	VkDescriptorSetAllocateInfo dsetalloci = {
		.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool     = pipeln->dpool,
		.descriptorSetCount = 1,
		.pSetLayouts        = &pipeln->dset_layout,
	};
	if ((vk_err = vkAllocateDescriptorSets(logical_gpu, &dsetalloci, &pipeln->dsets[pipeln->dsetc++])))
		ERROR("[VK] Failed to allocate for %d descriptor sets\n\t\"%d\"", dsetalloci.descriptorSetCount, vk_err); // TODO: STRING_OF_VK_ERR
	else
		DEBUG(3, "[VK] Allocated %ld descriptor sets", pipeln->dset_cap);

	VkDescriptorSet dset = pipeln->dsets[pipeln->dsetc - 1];
	VkWriteDescriptorSet dset_write;
	int dset_writec;
	if (ubos && uboc) {
		dset_writec = 0;
		for (int i = 0; i < uboc; i++) {
			dset_write = (VkWriteDescriptorSet){
				.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet           = dset,
				.dstBinding       = dset_writec++,
				.dstArrayElement  = 0,
				.descriptorCount  = 1,
				.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pTexelBufferView = NULL,
				.pImageInfo       = NULL,
				.pBufferInfo      = &(VkDescriptorBufferInfo){
					.buffer = ubos[i].buf,
					.offset = 0,
					.range  = ubos[i].sz,
				},
			};

			vkUpdateDescriptorSets(logical_gpu, 1, &dset_write, 0, NULL);
		}
	}
	if (sbos && sboc) {
		dset_writec = 10;
		for (int i = 0; i < sboc; i++) {
			dset_write = (VkWriteDescriptorSet){
				.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet           = dset,
				.dstBinding       = dset_writec++,
				.dstArrayElement  = 0,
				.descriptorCount  = 1,
				.descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.pTexelBufferView = NULL,
				.pImageInfo       = NULL,
				.pBufferInfo      = &(VkDescriptorBufferInfo){
					.buffer = sbos[i].buf,
					.offset = 0,
					.range  = sbos[i].sz,
				},
			};

			vkUpdateDescriptorSets(logical_gpu, 1, &dset_write, 0, NULL);
		}
	}
	if (img_views && img_viewc) {
		dset_writec = 20;
		dset_write = (VkWriteDescriptorSet){
			.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet          = dset,
			.dstBinding      = dset_writec++,
			.descriptorCount = 1,
			.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
			.pImageInfo      = &(VkDescriptorImageInfo){
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.sampler     = pipeln->sampler? pipeln->sampler: default_sampler,
			},
		};
		vkUpdateDescriptorSets(logical_gpu, 1, &dset_write, 0, NULL);

		for (int i = 0; i < img_viewc; i++) {
			dset_write = (VkWriteDescriptorSet){
				.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet          = dset,
				.dstBinding      = dset_writec++,
				.descriptorCount = 1,
				.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
				.pImageInfo      = &(VkDescriptorImageInfo){
					.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					.imageView   = img_views[i],
				},
			};
			vkUpdateDescriptorSets(logical_gpu, 1, &dset_write, 0, NULL);
		}
	}

	return dset;
}

void pipeln_free(struct Pipeline* pipeln)
{
	DEBUG(3, "[VK] Destroying pipeline (%p)...", (void*)pipeln);
	vkDestroyPipeline(logical_gpu, pipeln->pipeln, NULL);
	vkDestroyPipelineLayout(logical_gpu, pipeln->layout, NULL);
	vkDestroyDescriptorSetLayout(logical_gpu, pipeln->dset_layout, NULL);
	vkDestroyDescriptorPool(logical_gpu, pipeln->dpool, NULL);

	free(pipeln->dsets);
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

	DEBUG(4, "[GFX] Initialized %d shaders with %ld buffers (UBOs: %ld; SBOs: %ld) and %ld images",
	      stageic, pipeln->uboc + pipeln->sboc, pipeln->uboc, pipeln->sboc, pipeln->imgc);
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
