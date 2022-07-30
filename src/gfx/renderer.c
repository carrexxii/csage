#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "cglm/cglm.h"

#include "util/iarray.h"
#include "config.h"
#include "vulkan.h"
#include "buffers.h"
#include "device.h"
#include "image.h"
#include "pipeline.h"
#include "swapchain.h"
#include "model.h"
#include "vertices.h"
#include "camera.h"
#include "renderer.h"

static void record_command(uint imgi);
inline static void update_buffers();
static void create_framebuffers();
static void create_command_buffers();
static void create_sync_objects();

static uint frame;
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

static struct Lighting {
	vec3   sundir;
	float  ambient;
	float  sunpower;
	uint16 lightc;
	vec4   lights[RENDERER_MAX_LIGHTS];
} lighting;

uint16* renmdlc;
uint16* renlightc;
mat4*   renmats;
vec4*   renlights;
struct Model* renmdls;
static SBO matbuf;

void renderer_init(SDL_Window* win)
{
	vulkan_init(win);
	swapchain_init(surface, WINDOW_WIDTH, WINDOW_HEIGHT);

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

	matbuf = create_sbo(sizeof(float[16])*RENDERER_MAX_OBJECTS);

	pipeln.vshader = vulkan_new_shader(SHADER_DIR "shader.vert");
	pipeln.fshader = vulkan_new_shader(SHADER_DIR "shader.frag");
	pipeln.vbindc  = 1;
	pipeln.vbinds  = vertbinds;
	pipeln.vattrc  = ARRAY_LEN(vertattrs);
	pipeln.vattrs  = vertattrs;
	pipeln.uboc    = 2;
	pipeln.uboszs  = (uintptr[]){ sizeof(float[16]), sizeof(lighting) + RENDERER_MAX_LIGHTS*sizeof(vec4) };
	pipeln.sbo     = matbuf;
	pipeln.sbosz   = RENDERER_MAX_OBJECTS*sizeof(float[16]);
	pipeln_init(&pipeln, renderpass);

	memcpy(lighting.sundir, (float[]){ -1.0, 0.25, 5.0 }, sizeof(float[3]));
	glm_vec3_normalize(lighting.sundir);
	lighting.ambient  = 0.01;
	lighting.sunpower = 0.5;
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

void renderer_add_light(vec4 light)
{
	if (lighting.lightc >= RENDERER_MAX_LIGHTS)
		ERROR("[GFX] Maximum number of lights reached");
	memcpy(lighting.lights[lighting.lightc++], light, sizeof(vec4));
	DEBUG(3, "[GFX] Added light [%.4f %.4f %.4f] :: %.4f", light[0], light[1], light[2], light[3]);
}

void renderer_set_lights(uint16 lightc, vec4* lights)
{
	if (lightc > RENDERER_MAX_LIGHTS)
		ERROR("[GFX] %hu exceeds the maximum number of lights (%d)", lightc, RENDERER_MAX_LIGHTS);
	memcpy(lighting.lights, lights, lightc*sizeof(vec4));
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

	pipeln_free(&pipeln);

	DEBUG(3, "[VK] Destroying render pass...");
	vkDestroyRenderPass(gpu, renderpass, alloccb);

	vulkan_free();

	free(framebufs);
	free(cmdbufs);
}

/* Records a command for the given image
 *   imgi - index of the framebuffer image to record the command for
 */
static void record_command(uint imgi)
{
	update_buffers();
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

	vkCmdBindPipeline(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindDescriptorSets(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, &pipeln.dset, 0, NULL);

	for (uint i = 0; i < *renmdlc; i++) {
		vkCmdBindVertexBuffers(cmdbuf, 0, 1, &renmdls[i].vbo.buf, (VkDeviceSize[]) { 0 });
		vkCmdDraw(cmdbuf, renmdls[i].vertc, 1, 0, i);
	}

	vkCmdEndRenderPass(cmdbuf);
	if (vkEndCommandBuffer(cmdbuf) != VK_SUCCESS)
		ERROR("[VK] Failed to record comand buffer");
}

inline static void update_buffers()
{
	void*   mem;
	uintptr memsz = (*renmdlc)*sizeof(float[16]);
	vkMapMemory(gpu, pipeln.sbo.mem, 0, memsz, 0, &mem);
	memcpy(mem, renmats, memsz);
	vkUnmapMemory(gpu, pipeln.sbo.mem);

	mat4 vp;
	get_cam_vp(vp);
	update_buffer(pipeln.ubos[0], sizeof(mat4), vp);

	update_buffer(pipeln.ubos[1], sizeof(struct Lighting), &lighting);
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
	for (uint i = 0; i < FRAMES_IN_FLIGHT; i++)
		if (vkCreateSemaphore(gpu, &semai, alloccb, &semas.renderdone[i]) != VK_SUCCESS ||
			vkCreateSemaphore(gpu, &semai, alloccb, &semas.imgavail[i]) != VK_SUCCESS ||
			vkCreateFence(gpu, &fencei, alloccb, &fences.frames[i]) != VK_SUCCESS)
			ERROR("[VK] Failed to create sync object(s) for frame %d", i);
	DEBUG(3, "[VK] Created %d sync objects", FRAMES_IN_FLIGHT);
}
