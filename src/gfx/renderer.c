#include "vulkan/vulkan.h"
#include "SDL3/SDL.h"

#include "config.h"
#include "vulkan.h"
#include "buffers.h"
#include "device.h"
#include "image.h"
#include "pipeline.h"
#include "swapchain.h"
#include "camera.h"
#include "ui/ui.h"
#include "model.h"
#include "particles.h"
#include "maths/scratch.h"
#include "renderer.h"

#define FRAMES_IN_FLIGHT 2

VkSampler default_sampler;
Vec4 global_light;
Vec4 global_ambient;
UBO  global_light_ubo;

static void record_commands(int imgi, struct Camera* cam);
static void create_frame_buffers(void);
static void create_command_buffers(void);
static void create_sync_objects(void);

static void (*draw_fns[RENDERER_MAX_DRAW_FUNCTIONS])(VkCommandBuffer, struct Camera*);
static int draw_fnc = 0;
static int frame    = 0;
static struct {
	VkSemaphore renderdone[FRAMES_IN_FLIGHT];
	VkSemaphore imgavail[FRAMES_IN_FLIGHT];
} semas;
static struct {
	VkFence frames[FRAMES_IN_FLIGHT];
} fences;

static VkRenderPass     renderpass;
static VkFramebuffer*   frame_bufs;
static VkCommandBuffer* cmd_bufs;
// static struct Light     lights[RENDERER_MAX_LIGHTS];

VkRenderPass renderer_init()
{
	default_sampler = image_new_sampler(VK_FILTER_NEAREST);
	swapchain_init(surface, global_config.winw, global_config.winh);

	VkAttachmentDescription colour_attach = {
		.format         = swapchain.fmt.format,
		.samples        = gpu_properties.max_samples,
		.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
	VkAttachmentDescription depth_attach = {
		.format         = VK_FORMAT_D16_UNORM,
		.samples        = gpu_properties.max_samples,
		.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
	VkAttachmentDescription resolve_attach = {
		.format         = swapchain.fmt.format,
		.samples        = VK_SAMPLE_COUNT_1_BIT,
		.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};
	VkSubpassDescription subpass_desc = {
		.flags                = 0,
		.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.inputAttachmentCount = 0,
		.pInputAttachments    = NULL,
		.colorAttachmentCount = 1,
		.pColorAttachments    = (VkAttachmentReference[]){
			{ .attachment = 0,
			  .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, },
		},
		.pDepthStencilAttachment = (VkAttachmentReference[]){
			{ .attachment = 1,
			  .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, },
		},
		.pResolveAttachments = (VkAttachmentReference[]){
			{ .attachment = 2,
			  .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, },
		},
		.preserveAttachmentCount = 0,
		.pPreserveAttachments    = NULL,
	};
	VkRenderPassCreateInfo renpassi = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 3,
		.pAttachments    = (VkAttachmentDescription[]){ colour_attach, depth_attach, resolve_attach },
		.subpassCount    = 1,
		.pSubpasses      = &subpass_desc,
		.dependencyCount = 1,
		.pDependencies   = (VkSubpassDependency[]){
			{ .srcSubpass    = VK_SUBPASS_EXTERNAL,
			  .dstSubpass    = 0,
			  .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			  .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			  .srcAccessMask = 0,
			  .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
			                   VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, },
		},
	};
	if (vkCreateRenderPass(logical_gpu, &renpassi, NULL, &renderpass))
		ERROR("[VK] Failed to create render pass");
	else
		DEBUG(1, "[VK] Created render pass");

	create_frame_buffers();
	create_command_buffers();
	create_sync_objects();

	global_light_ubo = ubo_new(sizeof(Vec4[2]));

	return renderpass;
}

void renderer_clear_draw_list(void)
{
	draw_fnc = 0;
}

void renderer_add_to_draw_list(void (*fn)(VkCommandBuffer, struct Camera*))
{
	if (draw_fnc >= RENDERER_MAX_DRAW_FUNCTIONS)
		ERROR("[GFX] Max draw function count (%d/%d) exceeded", draw_fnc, RENDERER_MAX_DRAW_FUNCTIONS);
	draw_fns[draw_fnc++] = fn;
}

void renderer_set_global_lighting(Vec3 light, float power, Vec3 ambient_colour, float ambient_power)
{
	Vec3 dir    = normalized(light);
	Vec3 colour = normalized(ambient_colour);

	global_light   = (Vec4){ UNPACK3(dir.arr)   , power         };
	global_ambient = (Vec4){ UNPACK3(colour.arr), ambient_power };
	buffer_update(global_light_ubo, sizeof(Vec4[2]), (Vec4[]){ global_light, global_ambient }, 0);
}

void renderer_draw(struct Camera* cam)
{
	vkWaitForFences(logical_gpu, 1, &fences.frames[frame], true, UINT64_MAX);
	vkResetFences(logical_gpu, 1, &fences.frames[frame]);

	/* TODO: recreate swapchain for out of date */
	uint imgi;
	if ((vk_err = vkAcquireNextImageKHR(logical_gpu, swapchain.swapchain, UINT64_MAX, semas.imgavail[frame], NULL, &imgi)))
		ERROR("[VK] Failed to aquire next swapchain image: \n\t\"%d\"", vk_err);

	vkResetCommandBuffer(cmd_bufs[frame], 0);
	record_commands(imgi, cam);

	VkSubmitInfo submiti = {
		.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount   = 1,
		.signalSemaphoreCount = 1,
		.commandBufferCount   = 1,
		.pWaitSemaphores      = &semas.imgavail[frame],
		.pSignalSemaphores    = &semas.renderdone[frame],
		.pCommandBuffers      = &cmd_bufs[frame],
		.pWaitDstStageMask    = (VkPipelineStageFlags[]){
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		},
	};
	if ((vk_err = vkQueueSubmit(graphicsq, 1, &submiti, fences.frames[frame])))
		ERROR("[VK] Failed to submit draw command: \n\t\"%d\"", vk_err);

	VkPresentInfoKHR presi = {
		.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores    = &semas.renderdone[frame],
		.swapchainCount     = 1,
		.pSwapchains        = (VkSwapchainKHR[]){ swapchain.swapchain },
		.pImageIndices      = (uint32[]){ imgi },
		.pResults           = NULL,
	};
	if ((vk_err = vkQueuePresentKHR(presentq, &presi)))
		ERROR("[VK] Failed to present queue:\n\t\"%d\"", vk_err);

	frame = (frame + 1) % FRAMES_IN_FLIGHT;
}

void renderer_free()
{
	vkDeviceWaitIdle(logical_gpu);

	DEBUG(3, "[VK] Destroying sync objects...");
	for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(logical_gpu, semas.imgavail[i], NULL);
		vkDestroySemaphore(logical_gpu, semas.renderdone[i], NULL);
		vkDestroyFence(logical_gpu, fences.frames[i], NULL);
	}

	swapchain_free();
	vkDestroySampler(logical_gpu, default_sampler, NULL);

	DEBUG(3, "[VK] Destroying frambuffers...");
	for (uint i = 0; i < swapchain.imgc; i++)
		vkDestroyFramebuffer(logical_gpu, frame_bufs[i], NULL);

	// DEBUG(3, "[VK] Freeing UI pipeline...");
	// ui_free();
	DEBUG(3, "[VK] Freeing models pipeline...");
	models_free();
	// DEBUG(3, "[VK] Freeing font pipeline...");
	// font_free();
	// DEBUG(3, "[VK] Freeing particles pipeline...");
	// particles_free();

	ubo_free(&global_light_ubo);

	DEBUG(3, "[VK] Destroying render pass...");
	vkDestroyRenderPass(logical_gpu, renderpass, NULL);

	free(frame_bufs);
	free(cmd_bufs);
}

/* -------------------------------------------------------------------- */

/* Records a command for the given image
 *   imgi - index of the framebuffer image to record the command for
 */
static void record_commands(int imgi, struct Camera* cam)
{
	VkCommandBuffer cmd_buf = cmd_bufs[frame];
	VkCommandBufferBeginInfo begini = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = NULL,
	};
	if ((vk_err = vkBeginCommandBuffer(cmd_buf, &begini)))
		ERROR("[VK] Failed to begin command buffer for image %u: \n\t\"%d\"", imgi, vk_err);

	VkRenderPassBeginInfo renderpassi = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass      = renderpass,
		.framebuffer     = frame_bufs[imgi],
		.clearValueCount = 2,
		.pClearValues    = (VkClearValue[]){
			(VkClearValue){ .color = { 0.1, 0.1, 0.28, 0.0 } },
			(VkClearValue){ .depthStencil = { 1.0, 0 } },
		},
		.renderArea = {
			.offset = { 0, 0 },
			.extent = swapchain.ext,
		},
	};

	vkCmdBeginRenderPass(cmd_buf, &renderpassi, VK_SUBPASS_CONTENTS_INLINE);

	for (int i = 0; i < draw_fnc; i++)
		draw_fns[i](cmd_buf, cam);

	vkCmdEndRenderPass(cmd_buf);
	if (vkEndCommandBuffer(cmd_buf))
		ERROR("[VK] Failed to record comand buffer");
}

static void create_frame_buffers()
{
	frame_bufs = smalloc(swapchain.imgc*sizeof(VkFramebuffer));
	for (uint i = 0; i < swapchain.imgc; i++) {
		VkFramebufferCreateInfo framebufi = {
			.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass      = renderpass,
			.attachmentCount = 3,
			.pAttachments    = (VkImageView[]){ resolve_img.view, depth_img.view, swapchain.imgs[i].view },
			.width           = swapchain.ext.width,
			.height          = swapchain.ext.height,
			.layers          = 1,
		};
		if (vkCreateFramebuffer(logical_gpu, &framebufi, NULL, &frame_bufs[i]))
			ERROR("[VK] Failed to create framebuffer %u", i);
		else
			DEBUG(3, "[VK] Created framebuffer %u", i);
	}
}

static void create_command_buffers()
{
	cmd_bufs = smalloc(swapchain.imgc*sizeof(VkCommandBuffer));
	VkCommandBufferAllocateInfo bufi = {
		.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = swapchain.imgc,
		.commandPool        = cmd_pool,
	};
	if ((vk_err = vkAllocateCommandBuffers(logical_gpu, &bufi, cmd_bufs)))
		ERROR("[VK] Failed to create command buffers\n\t\"%d\"", vk_err);
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
		if (vkCreateSemaphore(logical_gpu, &semai, NULL, &semas.renderdone[i]) ||
		    vkCreateSemaphore(logical_gpu, &semai, NULL, &semas.imgavail[i]) ||
		    vkCreateFence(logical_gpu, &fencei, NULL, &fences.frames[i]))
			ERROR("[VK] Failed to create sync object(s) for frame %d", i);
	DEBUG(3, "[VK] Created %d sync objects", FRAMES_IN_FLIGHT);
}

