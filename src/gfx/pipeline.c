#include <vulkan/vulkan.h>

#include "config.h"
#include "vulkan.h"
#include "device.h"
#include "buffers.h"
#include "image.h"
#include "swapchain.h"
#include "renderer.h"
#include "pipeline.h"

static int init_shaders(struct Pipeline* pipeln, VkPipelineShaderStageCreateInfo* stageis);

void pipeln_init(struct Pipeline* pipeln, VkRenderPass renpass)
{
	VkPipelineShaderStageCreateInfo stagesci[5];
	int stagec = init_shaders(pipeln, stagesci);

	VkPipelineVertexInputStateCreateInfo vinci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pVertexBindingDescriptions      = pipeln->vbinds,
		.vertexBindingDescriptionCount   = pipeln->vbindc,
		.pVertexAttributeDescriptions    = pipeln->vattrs,
		.vertexAttributeDescriptionCount = pipeln->vattrc,
	};
	VkPipelineInputAssemblyStateCreateInfo inassci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = false,
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
		.offset = {0, 0},
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
		.blendEnable  = false,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
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
		.pSetLayouts            = &pipeln->dsetlay,
		.pushConstantRangeCount = pipeln->pushsz > 0? 1: 0,
		.pPushConstantRanges    = &tpushr,
	};
	if (vkCreatePipelineLayout(gpu, &tlaysci, alloccb, &pipeln->lay) != VK_SUCCESS)
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
		.layout              = pipeln->lay,
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
	DEBUG(2, "[VK] Destroying pipeline (%p)...", (void*)pipeln);
	vkDestroyPipeline(gpu, pipeln->pipeln, alloccb);
	vkDestroyPipelineLayout(gpu, pipeln->lay, alloccb);
	vkDestroyDescriptorPool(gpu, pipeln->dpool, alloccb);

	DEBUG(3, "[VK] Destroying shader modules...");
	if (pipeln->vshader)  vkDestroyShaderModule(gpu, pipeln->vshader , alloccb);
	if (pipeln->tcshader) vkDestroyShaderModule(gpu, pipeln->tcshader, alloccb);
	if (pipeln->teshader) vkDestroyShaderModule(gpu, pipeln->teshader, alloccb);
	if (pipeln->gshader)  vkDestroyShaderModule(gpu, pipeln->gshader , alloccb);
	if (pipeln->fshader)  vkDestroyShaderModule(gpu, pipeln->fshader , alloccb);

	DEBUG(3, "[VK] Destroying descriptor set...");
	vkDestroyDescriptorSetLayout(gpu, pipeln->dsetlay, alloccb);

	for (uint i = 0; i < pipeln->uboc; i++)
		free_ubo(pipeln->ubos[i]);
}

/* TODO: Tesselation shaders */
static int init_shaders(struct Pipeline* pipeln, VkPipelineShaderStageCreateInfo* stageis)
{
	int stageic = 0;
	if (pipeln->vshader)
		stageis[stageic++] = (VkPipelineShaderStageCreateInfo){
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage  = VK_SHADER_STAGE_VERTEX_BIT,
			.module = pipeln->vshader,
			.pName  = "main",
		};
	if (pipeln->gshader)
		stageis[stageic++] = (VkPipelineShaderStageCreateInfo){
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_GEOMETRY_BIT,
			.module = pipeln->gshader,
			.pName = "main",
		};
	if (pipeln->fshader)
		stageis[stageic++] = (VkPipelineShaderStageCreateInfo){
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = pipeln->fshader,
			.pName = "main",
		};

	/* Probably don't need to do this stuff */
	uintptr* uboszs = pipeln->uboszs;
	pipeln->uboszs  = scalloc(pipeln->uboc, sizeof(uintptr));
	memcpy(pipeln->uboszs, uboszs, pipeln->uboc*sizeof(uintptr));

	pipeln->ubos = scalloc(pipeln->uboc, sizeof(UBO));
	VkDescriptorSetLayoutBinding ubolays[pipeln->uboc];
	for (uint i = 0; i < pipeln->uboc; i++) {
		pipeln->ubos[i] = create_ubo(pipeln->uboszs[i]);
		ubolays[i] = (VkDescriptorSetLayoutBinding){
			.binding            = i,
			.stageFlags         = VK_SHADER_STAGE_ALL,
			.descriptorCount    = 1,
			.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pImmutableSamplers = NULL,
		};
	}
	VkDescriptorSetLayoutBinding samplerlay = {
		.binding            = pipeln->uboc,
		.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
		.descriptorCount    = 1,
		.descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLER,
		.pImmutableSamplers = NULL,
	};
	VkDescriptorSetLayoutBinding imagelay = {
		.binding            = pipeln->uboc + 1,
		.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
		.descriptorCount    = 1,
		.descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.pImmutableSamplers = NULL,
	};

	uint dsetbc = 0;
	VkDescriptorSetLayoutBinding dsetbs[pipeln->uboc + 2];
	for (uint i = 0; i < pipeln->uboc; i++)
		dsetbs[dsetbc++] = ubolays[i];
	dsetbs[dsetbc++] = samplerlay;
	dsetbs[dsetbc++] = imagelay;
	VkDescriptorSetLayoutCreateInfo dsetci = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = dsetbc,
		.pBindings    = dsetbs,
	};
	if (vkCreateDescriptorSetLayout(gpu, &dsetci, alloccb, &pipeln->dsetlay))
		ERROR("[VK] Failed to create descriptor set layout");
	else
		DEBUG(3, "[VK] Created descriptor set layout");
	VkDescriptorPoolCreateInfo dpoolci = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.poolSizeCount = 3,
		.pPoolSizes    = (VkDescriptorPoolSize[]){
			{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			 .descriptorCount = pipeln->uboc, },
			{.type = VK_DESCRIPTOR_TYPE_SAMPLER,
			 .descriptorCount = 1, },
			{.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			 .descriptorCount = 1, },
		},
		.maxSets = 1,
	};
	if (vkCreateDescriptorPool(gpu, &dpoolci, alloccb, &pipeln->dpool))
		ERROR("[VK] Failed to create descriptor pool");
	else
		DEBUG(3, "[VK] Created descriptor pool");

	VkDescriptorSetAllocateInfo dsalloci = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool     = pipeln->dpool,
		.descriptorSetCount = 1,
		.pSetLayouts        = &pipeln->dsetlay,
	};
	if (vkAllocateDescriptorSets(gpu, &dsalloci, &pipeln->dset) != VK_SUCCESS)
		ERROR("[VK] Failed to allocate for descriptor set");
	else
		DEBUG(3, "[VK] Allocated descriptor sets");

	/* !! Uniform buffers must be aligned as follows:
	 * - a float an alignement of 4
     * - a vec2 an alignement of 8
     * - a vec3, vec4, mat4 an alignement of 16
	 */
	VkDescriptorBufferInfo dbufis[pipeln->uboc];
	for (uint i = 0; i < pipeln->uboc; i++)
		dbufis[i] = (VkDescriptorBufferInfo){
			.buffer = pipeln->ubos[i].buf,
			.offset = 0,
			.range  = pipeln->uboszs[i],
		};
	VkDescriptorImageInfo dimgis[imagec + 1];
	dimgis[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	dimgis[0].imageView   = NULL;
	dimgis[0].sampler     = sampler;
	for (uint i = 0; i < imagec; i++) {
		dimgis[i + 1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		dimgis[i + 1].imageView   = imageviews[i];
		dimgis[i + 1].sampler     = NULL;
	};
	VkWriteDescriptorSet dwrites[pipeln->uboc + 2];
	for (uint i = 0; i < pipeln->uboc; i++)
		dwrites[i] = (VkWriteDescriptorSet){
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet           = pipeln->dset,
			.dstBinding       = i,
			.dstArrayElement  = 0,
			.descriptorCount  = 1,
			.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo      = &dbufis[i],
			.pImageInfo       = NULL,
			.pTexelBufferView = NULL,
		};
	dwrites[pipeln->uboc] = (VkWriteDescriptorSet){
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet           = pipeln->dset,
		.dstBinding       = pipeln->uboc,
		.dstArrayElement  = 0,
		.descriptorCount  = 1,
		.descriptorType   = VK_DESCRIPTOR_TYPE_SAMPLER,
		.pBufferInfo      = NULL,
		.pImageInfo       = &dimgis[0],
		.pTexelBufferView = NULL,
	};
	dwrites[pipeln->uboc + 1] = (VkWriteDescriptorSet){
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet           = pipeln->dset,
		.dstBinding       = pipeln->uboc + 1,
		.dstArrayElement  = 0,
		.descriptorCount  = imagec,
		.descriptorType   = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.pBufferInfo      = NULL,
		.pImageInfo       = &dimgis[1],
		.pTexelBufferView = NULL,
	};
	vkUpdateDescriptorSets(gpu, pipeln->uboc + imagec + 1, dwrites, 0, NULL);
	DEBUG(3, "[VK] Updated %u descriptor sets", 2 + imagec);

	return stageic;
}
