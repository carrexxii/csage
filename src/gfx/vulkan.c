#include <vulkan/vulkan.h>
#include "SDL3/SDL_vulkan.h"

#include "device.h"
#include "buffers.h"
#include "pipeline.h"
#include "swapchain.h"
#include "renderer.h"
#include "vulkan.h"

VkInstance   instance;
VkSurfaceKHR surface;

static VkDebugReportCallbackEXT dbg_cb;

[[noreturn]]
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_cb(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT obj_type,
                                               uint64 obj, uintptr location, int32 msg_code, const char* layer_prefix,
                                               const char* msg, void* user_data)
{
	(void)user_data;
	ERROR("\n%s - %s [Object: %ld] for a %s at %zu [%d]: \n\t\"%s\"", layer_prefix, STRING_OF_DEBUG_REPORT_OBJECT(flags), obj,
	      STRING_OF_DEBUG_REPORT_OBJECT(obj_type), location, msg_code, msg);
	exit(1);
}

void init_vulkan(SDL_Window* win)
{
	INFO(TERM_DARK_GREEN "[INIT] Initializing Vulkan...");
	VkApplicationInfo appi = {
		.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName   = APPLICATION_NAME,
		.applicationVersion = ENGINE_VERSION,
		.pEngineName        = ENGINE_NAME,
		.engineVersion      = APPLICATION_VERSION,
		.apiVersion         = VULKAN_API_VERSION,
	};

	uint32 extpc;
	vkEnumerateInstanceExtensionProperties(NULL, &extpc, NULL);
	VkExtensionProperties extps[extpc];
	vkEnumerateInstanceExtensionProperties(NULL, &extpc, extps);
	INFO(TERM_DARK_GREEN "[VK] %u Vulkan extensions are supported:", extpc);
	for (uint i = 0; i < extpc; i++)
		INFO("\t%s", extps[i].extensionName);

	/* Debugging exts must come last */
#ifdef _WIN32
	const char* exts[4] = { "VK_KHR_surface", "VK_KHR_win32_surface",
	                        "VK_EXT_debug_report", "VK_EXT_debug_utils" };
#else
	const char* exts[5] = { "VK_KHR_surface", "VK_KHR_xcb_surface", "VK_KHR_xlib_surface",
	                        "VK_EXT_debug_report", "VK_EXT_debug_utils" };
#endif
#ifdef DEBUG
	uint extc = 5;
#else
	uint extc = 2;
#endif

	uint32 vlc;
	vkEnumerateInstanceLayerProperties(&vlc, NULL);
	VkLayerProperties vl[vlc];
	vkEnumerateInstanceLayerProperties(&vlc, vl);
	INFO(TERM_DARK_GREEN "[VK] %u Vulkan validation layers available: ", vlc);
	for (uint i = 0; i < vlc; i++)
		INFO("\t%s", vl[i].layerName);

	INFO(TERM_DARK_GREEN "[VK] %d extensions enabled:", extc);
	for (uint i = 0; i < extc; i++)
		INFO("\t%s", exts[i]);

	VkInstanceCreateInfo instci = {
		.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.flags                   = 0,
		.pApplicationInfo        = &appi,
		.enabledExtensionCount   = extc,
		.ppEnabledExtensionNames = exts,
		.enabledLayerCount       = VULKAN_LAYER_COUNT,
		.ppEnabledLayerNames     = VULKAN_LAYERS,
	};
	if (vkCreateInstance(&instci, NULL, &instance) != VK_SUCCESS)
		ERROR("[VK] Failed to create Vulkan instance");
	else
		INFO(TERM_DARK_GREEN "[VK] Created Vulkan instance");

	/* Set up the callback for validation layer error messages */
#ifdef DEBUG
	VkDebugReportCallbackCreateInfoEXT debugi = {
		.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
		.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,
		.pfnCallback = debug_cb,
	};
	VK_GET_EXT(dbgfn, vkCreateDebugReportCallbackEXT);
	if (!dbgfn)
		ERROR("[VK] Failed to find debug extension callback function");
	dbgfn(instance, &debugi, NULL, &dbg_cb);
	INFO(TERM_DARK_GREEN "[VK] Created debug callback");
#endif

	if (!SDL_Vulkan_CreateSurface(win, instance, NULL, &surface))
		ERROR("[VK] SDL failed to create a surface (%s)", SDL_GetError());

	device_init_physical(instance, surface);
	device_init_logical(surface);
}

void vulkan_free()
{
#ifdef DEBUG
	VK_GET_EXT(dbgfn, vkDestroyDebugReportCallbackEXT);
	if (!dbgfn)
		ERROR("[VK] Failed to find debug extension callback destructor function");
	dbgfn(instance, dbg_cb, NULL);
#endif

	vkDestroySurfaceKHR(instance, surface, NULL);
	device_free();

	INFO(TERM_DARK_GREEN "[VK] Destroying Vulkan instance...");
	vkDestroyInstance(instance, NULL);
}

