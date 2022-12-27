#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "map/map.h"
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
inline static void buffer_updates();
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
static struct Pipeline  mdlpipeln;
static struct Pipeline  vxlpipeln;

static struct Lighting {
	vec3   sundir;
	float  ambient;
	float  sunpower;
	uint16 lightc;
	vec4   lights[RENDERER_MAX_LIGHTS];
} lighting;

mat4* renmats;
uint16*       renmdlc;
struct Model* renmdls;
uint16* renlightc;
vec4*   renlights;
static uint ubobufc;
static UBO ubobufs[8];
static SBO matbuf;
static SBO mapbuf;

void renderer_init()
{
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

	matbuf = sbo_new(RENDERER_MAX_OBJECTS * sizeof(float[16]));
	mapbuf = sbo_new(sizeof(struct MapDrawData));
	ubobufs[ubobufc++] = ubo_new(sizeof(float[16]));
	ubobufs[ubobufc++] = ubo_new(sizeof(lighting) + RENDERER_MAX_LIGHTS*sizeof(float[4]));

	mdlpipeln.vshader   = create_shader(SHADER_DIR "model.vert");
	mdlpipeln.fshader   = create_shader(SHADER_DIR "model.frag");
	mdlpipeln.vertbindc = 1;
	mdlpipeln.vertbinds = mdlvertbinds;
	mdlpipeln.vertattrc = ARRAY_LEN(mdlvertattrs);
	mdlpipeln.vertattrs = mdlvertattrs;
	mdlpipeln.uboc      = 2;
	mdlpipeln.ubos      = scalloc(2, sizeof(UBO));
	mdlpipeln.ubos[0]   = &ubobufs[0];
	mdlpipeln.ubos[1]   = &ubobufs[1];
	mdlpipeln.sbo       = &matbuf;
	mdlpipeln.sbosz     = RENDERER_MAX_OBJECTS * sizeof(float[16]);
	init_pipeln(&mdlpipeln, renderpass);

	vxlpipeln.topology  = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
	vxlpipeln.vshader   = create_shader(SHADER_DIR "voxel.vert");
	vxlpipeln.gshader   = create_shader(SHADER_DIR "voxel.geom");
	vxlpipeln.fshader   = create_shader(SHADER_DIR "voxel.frag");
	vxlpipeln.vertbindc = 1;
	vxlpipeln.vertbinds = vxlvertbinds;
	vxlpipeln.vertattrc = ARRAY_LEN(vxlvertattrs);
	vxlpipeln.vertattrs = vxlvertattrs;
	vxlpipeln.uboc      = 2;
	vxlpipeln.ubos      = scalloc(2, sizeof(UBO));
	vxlpipeln.ubos[0]   = &ubobufs[0];
	vxlpipeln.ubos[1]   = &ubobufs[1];
	vxlpipeln.sbo       = &mapbuf;
	vxlpipeln.sbosz     = sizeof(struct MapDrawData);
	init_pipeln(&vxlpipeln, renderpass);

	glm_vec3_copy((vec3){ 0.0, 100.0, -50.0 }, lighting.sundir);
	glm_vec3_normalize(lighting.sundir);
	lighting.ambient  = 0.03;
	lighting.sunpower = 2.0;
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
	glm_vec4_copy(light, lighting.lights[lighting.lightc++]);
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

	while (ubobufc--)
		ubo_free(&ubobufs[ubobufc]);
	free_pipeln(&mdlpipeln);
	free_pipeln(&vxlpipeln);

	DEBUG(3, "[VK] Destroying render pass...");
	vkDestroyRenderPass(gpu, renderpass, alloccb);

	free(framebufs);
	free(cmdbufs);
}

/* Records a command for the given image
 *   imgi - index of the framebuffer image to record the command for
 */
static void record_command(uint imgi)
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

	vkCmdBindPipeline(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, vxlpipeln.pipeln);
	vkCmdBindDescriptorSets(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, vxlpipeln.layout, 0, 1, &vxlpipeln.dset, 0, NULL);
	vkCmdBindVertexBuffers(cmdbuf, 0, 1, &map->verts.buf, (VkDeviceSize[]) { 0 });
	for (uint i = 0; i < map->indc; i++) {
		if (map->inds[i].zlvl != camzlvl || !map->inds[i].visible) {
			// DEBUG(1, "[%u] Skipping...", i);
			continue;
		}
		vkCmdBindIndexBuffer(cmdbuf, map->inds[i].ibo.buf, 0, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(cmdbuf, map->inds[i].indc, 1, 0, 0, i);
		// DEBUG(1, "[%u] Drawing %u indices", i, map->inds[i].indc);
	}

	vkCmdBindPipeline(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, mdlpipeln.pipeln);
	vkCmdBindDescriptorSets(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, mdlpipeln.layout, 0, 1, &mdlpipeln.dset, 0, NULL);
	for (uint i = 0; i < *renmdlc; i++) {
		vkCmdBindVertexBuffers(cmdbuf, 0, 1, &renmdls[i].vbo.buf, (VkDeviceSize[]) { 0 });
		vkCmdDraw(cmdbuf, renmdls[i].vertc, 1, 0, i);
	}

	vkCmdEndRenderPass(cmdbuf);
	if (vkEndCommandBuffer(cmdbuf) != VK_SUCCESS)
		ERROR("[VK] Failed to record comand buffer");
}

inline static void buffer_updates()
{
	void*   mem;
	uintptr memsz;

	memsz = (*renmdlc)*sizeof(mat4);
	vkMapMemory(gpu, mdlpipeln.sbo->mem, 0, memsz, 0, &mem);
	memcpy(mem, renmats, memsz);
	vkUnmapMemory(gpu, mdlpipeln.sbo->mem);

	mat4 vp;
	camera_get_vp(vp);
	buffer_update(*mdlpipeln.ubos[0], sizeof(mat4), vp);
	buffer_update(*mdlpipeln.ubos[1], sizeof(struct Lighting), &lighting);

	memsz = sizeof(struct MapDrawData);
	vkMapMemory(gpu, vxlpipeln.sbo->mem, 0, memsz, 0, &mem);
	memcpy(mem, &mapdd, memsz);
	vkUnmapMemory(gpu, vxlpipeln.sbo->mem);
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
