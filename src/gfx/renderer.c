#include <SDL2/SDL.h>
#include <cglm/mat4.h>
#include <vulkan/vulkan.h>

#include "util/iarray.h"
#include "config.h"
#include "vulkan.h"
#include "buffers.h"
#include "device.h"
#include "image.h"
#include "pipeline.h"
#include "swapchain.h"
#include "vertices.h"
#include "camera.h"
#include "font.h"
#include "renderer.h"

static void record_command(int imgi);
inline static void buffer_updates();
static void create_framebuffers();
static void create_command_buffers();
static void create_sync_objects();

static int frame;
static struct {
	VkSemaphore renderdone[FRAMES_IN_FLIGHT];
	VkSemaphore imgavail[FRAMES_IN_FLIGHT];
} semas;
static struct {
	VkFence frames[FRAMES_IN_FLIGHT];
} fences;

static VkRenderPass     renderpass;
static VkFramebuffer*   framebufs;
static VkCommandBuffer* cmdbufs;
static struct Pipeline  pipeln;

mat4 renmats[RENDERER_MAX_OBJECTS];
intptr       renmdlc = 0;
struct Model renmdls[RENDERER_MAX_OBJECTS];
static uint ubobufc;
static UBO ubobufs[8];
static SBO matbuf;

void renderer_init()
{
	swapchain_init(surface, config_window_width, config_window_height);

	VkAttachmentDescription colourdesc = {
		.format         = surfacefmt.format,
		.samples        = VK_SAMPLE_COUNT_1_BIT,
		.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};
	VkAttachmentDescription depthdesc = {
		.format         = VK_FORMAT_D16_UNORM,
		.samples        = VK_SAMPLE_COUNT_1_BIT,
		.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
	VkAttachmentReference colourref = {
		.attachment = 0,
		.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
	VkAttachmentReference depthref = {
		.attachment = 1,
		.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
	VkSubpassDescription subpassdesc = {
		.flags                   = 0,
		.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.inputAttachmentCount    = 0,
		.pInputAttachments       = NULL,
		.colorAttachmentCount    = 1,
		.pColorAttachments       = &colourref,
		.pResolveAttachments     = NULL,
		.pDepthStencilAttachment = &depthref,
		.preserveAttachmentCount = 0,
		.pPreserveAttachments    = NULL,
	};
	VkSubpassDependency subpassdep = {
		.srcSubpass    = VK_SUBPASS_EXTERNAL,
		.dstSubpass    = 0,
		.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
						 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	};
	VkRenderPassCreateInfo renpassi = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 2,
		.pAttachments    = (VkAttachmentDescription[]){ colourdesc, depthdesc },
		.subpassCount    = 1,
		.pSubpasses      = &subpassdesc,
		.dependencyCount = 1,
		.pDependencies   = &subpassdep,
	};
	if (vkCreateRenderPass(gpu, &renpassi, alloccb, &renderpass) != VK_SUCCESS)
		ERROR("[VK] Failed to create render pass");
	else
		DEBUG(1, "[VK] Created render pass");
	
	create_framebuffers();
	create_command_buffers();
	create_sync_objects();

	matbuf = sbo_new(RENDERER_MAX_OBJECTS*sizeof(mat4));
	ubobufs[ubobufc++] = ubo_new(sizeof(mat4));

	pipeln.vshader   = create_shader(SHADER_DIR "model.vert");
	pipeln.fshader   = create_shader(SHADER_DIR "model.frag");
	pipeln.vertbindc = 1;
	pipeln.vertbinds = mdlvertbinds;
	pipeln.vertattrc = ARRAY_LEN(mdlvertattrs);
	pipeln.vertattrs = mdlvertattrs;
	pipeln.uboc      = 1;
	pipeln.ubos      = ubobufs;
	pipeln.sbo       = &matbuf;
	pipeln.sbosz     = sizeof(mat4)*RENDERER_MAX_OBJECTS;
	init_pipeln(&pipeln, renderpass);

	font_init(renderpass);
	particles_init(renderpass);
}

// TODO: Fix removes
intptr renderer_add_model(struct Model mdl)
{
	renmdls[renmdlc] = mdl;
	return renmdlc++;
}

void renderer_draw()
{
	vkWaitForFences(gpu, 1, &fences.frames[frame], true, UINT64_MAX);
	vkResetFences(gpu, 1, &fences.frames[frame]);

	/* TODO: recreate swapchain for out of date */
	uint imgi;
	if (vkAcquireNextImageKHR(gpu, swapchain, UINT64_MAX, semas.imgavail[frame], NULL, &imgi) != VK_SUCCESS)
		ERROR("[VK] Failed to aquire next swapchain image");

	vkResetCommandBuffer(cmdbufs[frame], 0);
	record_command(imgi);

	VkSubmitInfo submiti = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount   = 1,
		.signalSemaphoreCount = 1,
		.commandBufferCount   = 1,
		.pWaitSemaphores      = &semas.imgavail[frame],
		.pSignalSemaphores    = &semas.renderdone[frame],
		.pCommandBuffers      = &cmdbufs[frame],
		.pWaitDstStageMask    = (VkPipelineStageFlags[]){
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		},
	};
	if (vkQueueSubmit(graphicsq, 1, &submiti, fences.frames[frame]) != VK_SUCCESS)
		ERROR("[VK] Failed to submit draw command");

	VkPresentInfoKHR presi = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores    = &semas.renderdone[frame],
		.swapchainCount     = 1,
		.pSwapchains        = (VkSwapchainKHR[]){ swapchain },
		.pImageIndices      = (uint32[]){ imgi },
		.pResults           = NULL,
	};
	if (vkQueuePresentKHR(presentq, &presi) != VK_SUCCESS)
		ERROR("[VK] Failed to present queue");

	frame = (frame + 1) % FRAMES_IN_FLIGHT;
}

void renderer_free()
{
	vkDeviceWaitIdle(gpu);

	DEBUG(3, "[VK] Destroying sync objects...");
	for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(gpu, semas.imgavail[i], alloccb);
		vkDestroySemaphore(gpu, semas.renderdone[i], alloccb);
		vkDestroyFence(gpu, fences.frames[i], alloccb);
	}

	image_free();
	swapchain_free();

	DEBUG(3, "[VK] Destroying frambuffers...");
	for (uint i = 0; i < swapchainimgc; i++)
		vkDestroyFramebuffer(gpu, framebufs[i], alloccb);

	while (ubobufc--)
		ubo_free(&ubobufs[ubobufc]);
	pipeln_free(&pipeln);
	font_free();
	particles_free();

	DEBUG(3, "[VK] Destroying render pass...");
	vkDestroyRenderPass(gpu, renderpass, alloccb);

	free(framebufs);
	free(cmdbufs);
}

/* Records a command for the given image
 *   imgi - index of the framebuffer image to record the command for
 */
static void record_command(int imgi)
{
	buffer_updates();
	VkCommandBuffer cmdbuf = cmdbufs[frame];
	VkCommandBufferBeginInfo begini = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = NULL,
	};
	if (vkBeginCommandBuffer(cmdbuf, &begini) != VK_SUCCESS)
		ERROR("[VK] Failed to begin command buffer for image %u", imgi);

	VkRenderPassBeginInfo renderpassi = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass      = renderpass,
		.framebuffer     = framebufs[imgi],
		.clearValueCount = 2,
		.pClearValues    = (VkClearValue[]){
			(VkClearValue){ .color = { 0.1, 0.1, 0.28, 1.0 } },
			(VkClearValue){ .depthStencil = { 1.0, 0 } },
		},
		.renderArea = {
			.offset = { 0, 0 },
			.extent = swapchainext,
		},
	};

	vkCmdBeginRenderPass(cmdbuf, &renderpassi, VK_SUBPASS_CONTENTS_INLINE);

	font_record_commands(cmdbuf);

	vkCmdBindPipeline(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindDescriptorSets(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, &pipeln.dset, 0, NULL);
	for (int i = 0; i < renmdlc; i++) {
		if (!renmdls[i].vertc)
			continue;
		// DEBUG(1, "[%d] Drawing %d vertices", i, renmdls[i].vertc);
		vkCmdBindVertexBuffers(cmdbuf, 0, 1, &renmdls[i].vbo.buf, (VkDeviceSize[]) { 0 });
		vkCmdDraw(cmdbuf, renmdls[i].vertc, 1, 0, i);
	}

	particles_record_commands(cmdbuf);

	vkCmdEndRenderPass(cmdbuf);
	if (vkEndCommandBuffer(cmdbuf) != VK_SUCCESS)
		ERROR("[VK] Failed to record comand buffer");
}

inline static void buffer_updates()
{
	void*  mem;
	intptr memsz;

	if (renmdlc) {
		memsz = renmdlc*sizeof(mat4);
		vkMapMemory(gpu, pipeln.sbo->mem, 0, memsz, 0, &mem);
		memcpy(mem, renmats, memsz);
		vkUnmapMemory(gpu, pipeln.sbo->mem);
	}

	mat4 vp;
	camera_get_vp(vp);
	buffer_update(pipeln.ubos[0], sizeof(mat4), vp);
}

static void create_framebuffers()
{
	framebufs = smalloc(swapchainimgc*sizeof(VkFramebuffer));
	for (uint i = 0; i < swapchainimgc; i++) {
		VkFramebufferCreateInfo framebufi = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass      = renderpass,
			.attachmentCount = 2,
			.pAttachments    = (VkImageView[]){ swapchainimgviews[i], depthview },
			.width           = swapchainext.width,
			.height          = swapchainext.height,
			.layers          = 1,
		};
		if (vkCreateFramebuffer(gpu, &framebufi, alloccb, &framebufs[i]))
			ERROR("[VK] Failed to create framebuffer %u", i);
		else
			DEBUG(3, "[VK] Created framebuffer %u", i);
	}
}

static void create_command_buffers()
{
	cmdbufs = smalloc(swapchainimgc*sizeof(VkCommandBuffer));
	VkCommandBufferAllocateInfo bufi = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = swapchainimgc,
		.commandPool        = cmdpool,
	};
	if (vkAllocateCommandBuffers(gpu, &bufi, cmdbufs))
		ERROR("[VK] Failed to create command buffers");
	else
		DEBUG(3, "[VK] Created command buffers");
}

static void create_sync_objects()
{
	frame = 0;
	VkSemaphoreCreateInfo semai = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};
	VkFenceCreateInfo fencei = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};
	for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
		if (vkCreateSemaphore(gpu, &semai, alloccb, &semas.renderdone[i]) != VK_SUCCESS ||
			vkCreateSemaphore(gpu, &semai, alloccb, &semas.imgavail[i]) != VK_SUCCESS ||
			vkCreateFence(gpu, &fencei, alloccb, &fences.frames[i]) != VK_SUCCESS)
			ERROR("[VK] Failed to create sync object(s) for frame %d", i);
	DEBUG(3, "[VK] Created %d sync objects", FRAMES_IN_FLIGHT);
}

