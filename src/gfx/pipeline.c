#include <vulkan/vulkan.h>

#include "resmgr.h"
#include "vulkan.h"
#include "buffers.h"
#include "swapchain.h"
#include "image.h"
#include "renderer.h"
#include "pipeline.h"

static int  init_shaders(PipelineCreateInfo* ci, VkPipelineShaderStageCreateInfo* stage_cis);
static void init_descriptors(Pipeline* pipeln, PipelineCreateInfo* ci);
static inline VkDescriptorSetLayoutBinding create_dset_layout(VkShaderStageFlagBits stage_flags, VkDescriptorType type, int binding);

static Pipeline pipelns[MAX_PIPELINES];
static VArray   free_pipelns;

// TODO: create all the pipelines at init and reuse from there
void pipelns_init()
{
	free_pipelns = varray_new(MAX_PIPELINES, sizeof(int));
	for (int i = 0; i < MAX_PIPELINES; i++)
		varray_push(&free_pipelns, &i);

	INFO(TERM_DARK_GREEN "[VK] Initialized pipelines (%d total)", MAX_PIPELINES);
}

void pipelns_free()
{
	if (free_pipelns.len != MAX_PIPELINES) {
		WARN("[VK] Not all pipelines have been free'd (%ld/%d)", free_pipelns.len, MAX_PIPELINES);
		for (int i = 0; i < MAX_PIPELINES; i++)
			if (!varray_contains(&free_pipelns, &i))
				fprintf(stderr, TERM_ORANGE "\t-> \"%s\"\n" TERM_NORMAL, pipelns[i].name);
	}
}

/* -------------------------------------------------------------------- */

Pipeline* pipeln_new(PipelineCreateInfo* ci, const char* name)
{
	if (free_pipelns.len < 1) {
		ERROR("[GFX] No more pipelines available (of total %d)", MAX_PIPELINES);
		return NULL;
	}

	DEFAULT(ci->topology         , VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	DEFAULT(ci->primitive_restart, false);
	DEFAULT(ci->sampler          , default_sampler);
	DEFAULT(ci->push_stages      , VK_SHADER_STAGE_ALL);

	int i = *(int*)varray_pop(&free_pipelns);
	Pipeline* atomic pipeln = &pipelns[i];
	if (pipeln->is_active)
		pipeln_free(pipeln);

	*pipeln = (Pipeline){
		.i         = i,
		.is_active = false,
		.name      = name,
	};

	INFO(TERM_DARK_GREEN "[VK] Created new pipeline \"%s\" ([%d] %p) with:"
	     TERM_NORMAL     "\n\t%d UBOs\n\t%d SBOs\n\t%d Images\n\tPush constants: %s (%dB) (0x%X stages)",
	     pipeln->name, pipeln->i, (void*)pipeln, ci->uboc, ci->sboc, ci->imgc, STR_TF(ci->push_sz), ci->push_sz, ci->push_stages);
	return pipeln;
}

Pipeline* pipeln_update(Pipeline* old_pipeln, PipelineCreateInfo* ci)
{
	Pipeline* pipeln;
	if (old_pipeln->is_active) {
		old_pipeln->is_active = false;
		resmgr_defer(RES_PIPELINE, old_pipeln);
		pipeln = pipeln_new(ci, old_pipeln->name);
	} else {
	 	pipeln = old_pipeln;
	}

	VkPipelineShaderStageCreateInfo stages_ci[5];
	int stagec = init_shaders(ci, stages_ci);
	if (stagec < 1) {
		ERROR("[VK] Pipeline does not have any stages (stage count = %d)", stagec);
		return NULL;
	}

	init_descriptors(pipeln, ci);

	VkPipelineVertexInputStateCreateInfo vert_input_ci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pVertexBindingDescriptions      = ci->vert_binds,
		.vertexBindingDescriptionCount   = ci->vert_bindc,
		.pVertexAttributeDescriptions    = ci->vert_attrs,
		.vertexAttributeDescriptionCount = ci->vert_attrc,
	};
	VkPipelineInputAssemblyStateCreateInfo input_ass_ci = {
		.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology               = ci->topology,
		.primitiveRestartEnable = ci->primitive_restart,
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
	VkPipelineViewportStateCreateInfo viewport_ci = {
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
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
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
		.blendConstants  = { 0.0f, 0.0f, 0.0f, 0.0f },
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
	VkPushConstantRange push_range = {
		.stageFlags = ci->push_stages,
		.offset     = 0,
		.size       = ci->push_sz,
	};
	VkPipelineLayoutCreateInfo tlaysci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount         = 1,
		.pSetLayouts            = &pipeln->dset.layout,
		.pushConstantRangeCount = !!ci->push_sz,
		.pPushConstantRanges    = &push_range,
	};
	if ((vk_err = vkCreatePipelineLayout(logical_gpu, &tlaysci, NULL, &pipeln->layout)))
		ERROR("[VK] Failed to create pipeline layout\n\t\"%s\"", STRING_OF_VK_RESULT(vk_err));
	else
		INFO(TERM_DARK_GREEN "[VK] Created pipeline layout");

	VkGraphicsPipelineCreateInfo tpipelnci = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount          = stagec,
		.pStages             = stages_ci,
		.pVertexInputState   = &vert_input_ci,
		.pInputAssemblyState = &input_ass_ci,
		.pViewportState      = &viewport_ci,
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
		ERROR("[VK] Failed to create pipeline\n\t\"%s\"", STRING_OF_VK_RESULT(vk_err));
	else
		INFO(TERM_DARK_GREEN "[VK] Pipeline updated");

	pipeln->is_active = true;
	return pipeln;
}

void pipeln_free(Pipeline* pipeln)
{
	vkDestroyDescriptorPool(logical_gpu, pipeln->dpool, NULL);
	vkDestroyDescriptorSetLayout(logical_gpu, pipeln->dset.layout, NULL);
	vkDestroyPipeline(logical_gpu, pipeln->pipeln, NULL);
	vkDestroyPipelineLayout(logical_gpu, pipeln->layout, NULL);

	pipeln->is_active = false;
	varray_push(&free_pipelns, &pipeln->i);
}

/* -------------------------------------------------------------------- */

// TODO: specialization constants
static int init_shaders(PipelineCreateInfo* ci, VkPipelineShaderStageCreateInfo* stage_cis)
{
	int stageic = 0;
	if (ci->vshader.data)
		stage_cis[stageic++] = (VkPipelineShaderStageCreateInfo){
			.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage  = VK_SHADER_STAGE_VERTEX_BIT,
			.module = load_shader(ci->vshader),
			.pName  = "main",
		};
	if (ci->tcshader.data)
		stage_cis[stageic++] = (VkPipelineShaderStageCreateInfo){
			.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
			.module = load_shader(ci->tcshader),
			.pName  = "main",
		};
	if (ci->teshader.data)
		stage_cis[stageic++] = (VkPipelineShaderStageCreateInfo){
			.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
			.module = load_shader(ci->teshader),
			.pName  = "main",
		};
	if (ci->gshader.data)
		stage_cis[stageic++] = (VkPipelineShaderStageCreateInfo){
			.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage  = VK_SHADER_STAGE_GEOMETRY_BIT,
			.module = load_shader(ci->gshader),
			.pName  = "main",
		};
	if (ci->fshader.data)
		stage_cis[stageic++] = (VkPipelineShaderStageCreateInfo){
			.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = load_shader(ci->fshader),
			.pName  = "main",
		};

	INFO(TERM_DARK_GREEN "[GFX] Initialized %d shaders with %d buffers (UBOs: %d; SBOs: %d) and %d images",
	      stageic, ci->uboc + ci->sboc, ci->uboc, ci->sboc, ci->imgc);
	return stageic;
}

static void init_descriptors(Pipeline* pipeln, PipelineCreateInfo* ci)
{
	/*** -------------------- Create the Descriptor Set Layout -------------------- ***/
	VkDescriptorSetLayoutBinding bindings[PIPELINE_MAX_BINDINGS];
	VkShaderStageFlagBits stages;
	int bindingc = 0;
	int binding  = 0;
	for (int i = 0; i < ci->uboc; i++)
		bindings[bindingc++] = create_dset_layout(ci->ubo_stages[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding++);

	binding = 10;
	for (int i = 0; i < ci->sboc; i++)
		bindings[bindingc++] = create_dset_layout(ci->sbo_stages[i], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, binding++);

	binding = 20;
	if (ci->imgc) {
		bindings[bindingc++] = create_dset_layout(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLER, binding++);
		for (int i = 0; i < ci->imgc; i++)
			bindings[bindingc++] = create_dset_layout(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, binding++);
	}

	VkDescriptorSetLayoutCreateInfo layout_ci = {
		.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = bindingc,
		.pBindings    = bindings,
	};
	if ((vk_err = vkCreateDescriptorSetLayout(logical_gpu, &layout_ci, NULL, &pipeln->dset.layout)))
		ERROR("[VK] Failed to create descriptor set layout (%d)", vk_err);
	else
		INFO(TERM_DARK_GREEN "[VK] Created descriptor set layout with %d descriptors", bindingc);

	/*** -------------------- Create the Descriptor Pool -------------------- ***/
	VkDescriptorPoolSize pool_szs[4];
	int dtypec = 0;
	if (ci->uboc)
		pool_szs[dtypec++] = (VkDescriptorPoolSize){
			.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = ci->uboc,
		};
	if (ci->sboc)
		pool_szs[dtypec++] = (VkDescriptorPoolSize){
			.type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = ci->sboc,
		};
	if (ci->imgc) {
		pool_szs[dtypec++] = (VkDescriptorPoolSize){
			.type            = VK_DESCRIPTOR_TYPE_SAMPLER,
			.descriptorCount = 1,
		};
		pool_szs[dtypec++] = (VkDescriptorPoolSize){
			.type            = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = ci->imgc,
		};
	}
	VkDescriptorPoolCreateInfo dpool_ci = {
		.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags         = 0,
		.poolSizeCount = dtypec,
		.pPoolSizes    = pool_szs,
		.maxSets       = 1,
	};
	if ((vk_err = vkCreateDescriptorPool(logical_gpu, &dpool_ci, NULL, &pipeln->dpool)))
		ERROR("[VK] Failed to create descriptor pool\n\t\"%s\"", STRING_OF_VK_RESULT(vk_err));
	else
		INFO(TERM_DARK_GREEN "[VK] Created descriptor pool");

	/*** -------------------- Allocate the Descriptor Set -------------------- ***/
	VkDescriptorSetAllocateInfo dset_alloci = {
		.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool     = pipeln->dpool,
		.descriptorSetCount = 1,
		.pSetLayouts        = &pipeln->dset.layout,
	};
	if ((vk_err = vkAllocateDescriptorSets(logical_gpu, &dset_alloci, &pipeln->dset.set)))
		ERROR("[VK] Failed to allocate descriptor set\n\t\"%s\"", STRING_OF_VK_RESULT(vk_err)); // TODO: STRING_OF_VK_ERR
	else
		INFO(TERM_DARK_GREEN "[VK] Allocated a descriptor set");

	/*** -------------------- Update the Descriptors -------------------- ***/
	VkWriteDescriptorSet dset_write;
	int i;
	binding = 0;
	for (i = 0; i < ci->uboc; i++) {
		dset_write = (VkWriteDescriptorSet){
			.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet           = pipeln->dset.set,
			.dstBinding       = binding++,
			.dstArrayElement  = 0,
			.descriptorCount  = 1,
			.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pTexelBufferView = NULL,
			.pImageInfo       = NULL,
			.pBufferInfo      = &(VkDescriptorBufferInfo){
				.buffer = ci->ubos[i].buf,
				.offset = 0,
				.range  = ci->ubos[i].sz,
			},
		};
		vkUpdateDescriptorSets(logical_gpu, 1, &dset_write, 0, NULL);
	}

	binding = 10;
	for (i = 0; i < ci->sboc; i++) {
		dset_write = (VkWriteDescriptorSet){
			.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet           = pipeln->dset.set,
			.dstBinding       = binding++,
			.dstArrayElement  = 0,
			.descriptorCount  = 1,
			.descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pTexelBufferView = NULL,
			.pImageInfo       = NULL,
			.pBufferInfo      = &(VkDescriptorBufferInfo){
				.buffer = ci->sbos[i].buf,
				.offset = 0,
				.range  = ci->sbos[i].sz,
			},
		};
		vkUpdateDescriptorSets(logical_gpu, 1, &dset_write, 0, NULL);
	}

	binding = 20;
	if (ci->imgc > 0) {
		dset_write = (VkWriteDescriptorSet){
			.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet          = pipeln->dset.set,
			.dstBinding      = binding++,
			.descriptorCount = 1,
			.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
			.pImageInfo      = &(VkDescriptorImageInfo){
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.sampler     = ci->sampler,
			},
		};
		vkUpdateDescriptorSets(logical_gpu, 1, &dset_write, 0, NULL);

		for (i = 0; i < ci->imgc; i++) {
			dset_write = (VkWriteDescriptorSet){
				.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet          = pipeln->dset.set,
				.dstBinding      = binding++,
				.descriptorCount = 1,
				.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
				.pImageInfo      = &(VkDescriptorImageInfo){
					.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					.imageView   = ci->imgs[i]->view,
				},
			};
			vkUpdateDescriptorSets(logical_gpu, 1, &dset_write, 0, NULL);
		}
	}
}

static inline VkDescriptorSetLayoutBinding create_dset_layout(VkShaderStageFlagBits stage_flags, VkDescriptorType type, int binding)
{
	DEFAULT(stage_flags, VK_SHADER_STAGE_ALL);
	return (VkDescriptorSetLayoutBinding){
		.binding            = binding,
		.stageFlags         = stage_flags,
		.descriptorCount    = 1,
		.descriptorType     = type,
		.pImmutableSamplers = NULL,
	};
}

