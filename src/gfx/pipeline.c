#include <limits.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "common.h"
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
inline static VkDescriptorSetLayoutBinding create_dset_layout(VkShaderStageFlagBits stagef, VkDescriptorType type,
                                                              uint binding);
inline static void update_buf_dset(VkWriteDescriptorSet* dwriteset, VkDescriptorSet dset, VkDescriptorType type,
                                   VkDescriptorBufferInfo* dbufi, uintptr binding);
inline static void update_img_dset(VkWriteDescriptorSet* dwriteset, VkDescriptorSet dset, VkDescriptorType type,
                                   VkDescriptorImageInfo* dimgi, uintptr binding);
static void update_dsets(struct Pipeline* pipeln);

void pipeln_init(struct Pipeline* pipeln, VkRenderPass renpass)
{
	VkPipelineShaderStageCreateInfo stagesci[5];
	int stagec = init_shaders(pipeln, stagesci);

	VkPipelineVertexInputStateCreateInfo vinci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pVertexBindingDescriptions      = pipeln->vertbinds,
		.vertexBindingDescriptionCount   = pipeln->vertbindc,
		.pVertexAttributeDescriptions    = pipeln->vertattrs,
		.vertexAttributeDescriptionCount = pipeln->vertattrc,
	};
	VkPipelineInputAssemblyStateCreateInfo inassci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = pipeln->topology? pipeln->topology: VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = pipeln->topology && pipeln->topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST? true: false,
	};
	VkViewport viewport = {
		.x        = 0.0,
		.y        = 0.0,
		.width    = (float)swapchainext.width,
		.height   = (float)swapchainext.height,
		.minDepth = 0.0,
		.maxDepth = 1.0,
	};
	VkRect2D scissor = {
		.offset = { 0, 0 },
		.extent = swapchainext,
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
		.frontFace               = VK_FRONT_FACE_CLOCKWISE,
		.depthClampEnable        = false,
		.depthBiasEnable         = false,
		.depthBiasClamp          = 0.0,
		.depthBiasConstantFactor = 0.0,
		.depthBiasSlopeFactor    = 0.0,
	};
	VkPipelineMultisampleStateCreateInfo msci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.sampleShadingEnable   = false,
		.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
		.minSampleShading      = 1.0,
		.pSampleMask           = NULL,
		.alphaToCoverageEnable = false,
		.alphaToOneEnable      = false,
	};
	VkPipelineColorBlendAttachmentState blendattachs = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
						  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable  = true,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA,
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
		.stageFlags = pipeln->pushstages,
		.offset     = 0,
		.size       = pipeln->pushsz,
	};
	VkPipelineLayoutCreateInfo tlaysci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount         = 1,
		.pSetLayouts            = &pipeln->dsetlayout,
		.pushConstantRangeCount = pipeln->pushsz > 0? 1: 0,
		.pPushConstantRanges    = &tpushr,
	};
	if (vkCreatePipelineLayout(gpu, &tlaysci, alloccb, &pipeln->layout) != VK_SUCCESS)
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
		.renderPass          = renpass,
		.subpass             = 0,
		.basePipelineHandle  = NULL,
		.basePipelineIndex   = -1,
	};
	if (vkCreateGraphicsPipelines(gpu, NULL, 1, &tpipelnci, alloccb, &pipeln->pipeln) != VK_SUCCESS)
		ERROR("[VK] Failed to create pipeline");
	else
		DEBUG(3, "[VK] Pipeline created");
}

void pipeln_free(struct Pipeline* pipeln)
{
	DEBUG(3, "[VK] Destroying pipeline (%p)...", (void*)pipeln);
	vkDestroyPipeline(gpu, pipeln->pipeln, alloccb);
	vkDestroyPipelineLayout(gpu, pipeln->layout, alloccb);
	vkDestroyDescriptorPool(gpu, pipeln->dpool, alloccb);

	if (pipeln->vshader)  vkDestroyShaderModule(gpu, pipeln->vshader , alloccb);
	if (pipeln->tcshader) vkDestroyShaderModule(gpu, pipeln->tcshader, alloccb);
	if (pipeln->teshader) vkDestroyShaderModule(gpu, pipeln->teshader, alloccb);
	if (pipeln->gshader)  vkDestroyShaderModule(gpu, pipeln->gshader , alloccb);
	if (pipeln->fshader)  vkDestroyShaderModule(gpu, pipeln->fshader , alloccb);

	vkDestroyDescriptorSetLayout(gpu, pipeln->dsetlayout, alloccb);

	for (int i = 0; i < pipeln->uboc; i++)
		if (pipeln->ubos[i].sz)
			ubo_free(&pipeln->ubos[i]);
	if (pipeln->sbo)
		sbo_free(pipeln->sbo);
}

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

	uint dsetlayoutc = 0;
	VkDescriptorSetLayoutBinding dsetlayouts[3 + pipeln->uboc]; /* 3 = storage buffer + sampler + images */
	if (pipeln->sbosz > 0) {
		dsetlayouts[dsetlayoutc] = create_dset_layout(VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, dsetlayoutc);
		dsetlayoutc++;
	}
	for (int i = 0; i < pipeln->uboc; i++) {
		dsetlayouts[dsetlayoutc] = create_dset_layout(VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, dsetlayoutc);
		dsetlayoutc++;
	}
	dsetlayouts[dsetlayoutc] = create_dset_layout(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLER, dsetlayoutc);
	dsetlayoutc++;
	dsetlayouts[dsetlayoutc] = create_dset_layout(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, dsetlayoutc);
	dsetlayoutc++;

	VkDescriptorSetLayoutCreateInfo dsetlayoutci = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = dsetlayoutc,
		.pBindings    = dsetlayouts,
	};
	if (vkCreateDescriptorSetLayout(gpu, &dsetlayoutci, alloccb, &pipeln->dsetlayout))
		ERROR("[VK] Failed to create descriptor set layout");
	else
		DEBUG(3, "[VK] Created descriptor set layout");

	VkDescriptorPoolCreateInfo dpoolci = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.poolSizeCount = 4, /* 4 = storage buffer + uniform buffers + sampler + sampled images */
		.pPoolSizes    = (VkDescriptorPoolSize[]){
			{ .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			  .descriptorCount = 1, },
			{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			  .descriptorCount = pipeln->uboc? pipeln->uboc: 1, },
			{ .type = VK_DESCRIPTOR_TYPE_SAMPLER,
			  .descriptorCount = 1, },
			{ .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			  .descriptorCount = 1, },
		},
		.maxSets = 1,
	};
	if (vkCreateDescriptorPool(gpu, &dpoolci, alloccb, &pipeln->dpool))
		ERROR("[VK] Failed to create descriptor pool");
	else
		DEBUG(3, "[VK] Created descriptor pool");

	VkDescriptorSetAllocateInfo dsetalloci = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool     = pipeln->dpool,
		.descriptorSetCount = 1,
		.pSetLayouts        = &pipeln->dsetlayout,
	};
	if (vkAllocateDescriptorSets(gpu, &dsetalloci, &pipeln->dset) != VK_SUCCESS)
		ERROR("[VK] Failed to allocate for descriptor set");
	else
		DEBUG(3, "[VK] Allocated descriptor sets");

	update_dsets(pipeln);

	return stageic;
}

inline static VkDescriptorSetLayoutBinding create_dset_layout(VkShaderStageFlagBits stagef, VkDescriptorType type,
                                                              uint binding)
{
	return (VkDescriptorSetLayoutBinding){
		.binding            = binding,
		.stageFlags         = stagef,
		.descriptorCount    = 1,
		.descriptorType     = type,
		.pImmutableSamplers = NULL,
	};
}

inline static void update_buf_dset(VkWriteDescriptorSet* dwriteset, VkDescriptorSet dset, VkDescriptorType type,
                                   VkDescriptorBufferInfo* dbufi, uintptr binding)
{
	if (!dbufi->buffer)
		ERROR("[VK] Should not have NULL buffer passed for descriptor set (binding: %lu)", binding);
	*dwriteset = (VkWriteDescriptorSet){
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet           = dset,
		.dstBinding       = binding,
		.dstArrayElement  = 0,
		.descriptorCount  = 1,
		.descriptorType   = type,
		.pBufferInfo      = dbufi,
		.pImageInfo       = NULL,
		.pTexelBufferView = NULL,
	};
}

inline static void update_img_dset(VkWriteDescriptorSet* dwriteset, VkDescriptorSet dset, VkDescriptorType type,
                                   VkDescriptorImageInfo* dimgi, uintptr binding)
{
	*dwriteset = (VkWriteDescriptorSet){
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet           = dset,
		.dstBinding       = binding,
		.dstArrayElement  = 0,
		.descriptorCount  = 1,
		.descriptorType   = type,
		.pBufferInfo      = NULL,
		.pImageInfo       = dimgi,
		.pTexelBufferView = NULL,
	};
}

static void update_dsets(struct Pipeline* pipeln)
{
	int sbo_loc, ubo_loc, sampler_loc, images_loc;
	sbo_loc = ubo_loc = sampler_loc = images_loc = -1;
	int dwritesetc = 0;
	VkWriteDescriptorSet   dwritesets[2 + pipeln->uboc + imagec]; /* 2 = storage buffer + sampler */
	VkDescriptorBufferInfo dbufis[1 + pipeln->uboc];              /* 1 = storage buffer           */
	VkDescriptorImageInfo  dimgis[1 + imagec];                    /* 1 = sampler                  */

	/* Storage buffers */
	if (pipeln->sbosz > 0) {
		dbufis[dwritesetc] = (VkDescriptorBufferInfo){
			.buffer = pipeln->sbo->buf,
			.offset = 0,
			.range  = pipeln->sbosz,
		};
		update_buf_dset(&dwritesets[dwritesetc], pipeln->dset, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		                &dbufis[dwritesetc], dwritesetc);
		sbo_loc = dwritesetc;
		dwritesetc++;
	}
 	
	/* Uniform buffers */
	if (pipeln->uboc > 0)
		ubo_loc = dwritesetc;
	for (int i = 0; i < pipeln->uboc; i++) {
		dbufis[dwritesetc] = (VkDescriptorBufferInfo){
			.buffer = pipeln->ubos[i].buf,
			.offset = 0,
			.range  = pipeln->ubos[i].sz,
		};
		update_buf_dset(&dwritesets[dwritesetc], pipeln->dset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
		                &dbufis[dwritesetc], dwritesetc);
		dwritesetc++;
	}

	/* Sampler */
	dimgis[0] = (VkDescriptorImageInfo){
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.imageView   = NULL,
		.sampler     = sampler,
	};
	update_img_dset(&dwritesets[dwritesetc], pipeln->dset, VK_DESCRIPTOR_TYPE_SAMPLER, &dimgis[0], dwritesetc);
	sampler_loc = dwritesetc;
	dwritesetc++;

	vkUpdateDescriptorSets(gpu, dwritesetc, dwritesets, 0, NULL);

	/* Sampled images */
	if (pipeln->texturec > 0)
		images_loc = dwritesetc;
	for (int i = 0; i < pipeln->texturec; i++) {
		dimgis[i + 1] = (VkDescriptorImageInfo){
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.imageView   = pipeln->textures[i].image_view,
			.sampler     = NULL,
		};
		update_img_dset(&dwritesets[dwritesetc], pipeln->dset, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		                &dimgis[i + 1], dwritesetc);
		dwritesetc++;
	}
	vkUpdateDescriptorSets(gpu, 1, &dwritesets[dwritesetc-1], 0, NULL);
	
	DEBUG(3, "[VK] Updated %d descriptor sets (SBO: %d; UBO: %d; Sampler: %d; Images: %d)", dwritesetc, 
	      sbo_loc, ubo_loc, sampler_loc, images_loc);
}

