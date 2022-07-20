#ifndef GFX_VULKAN_H
#define GFX_VULKAN_H

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

#include "device.h"
#include "pipeline.h"
#include "buffers.h"

#define VULKAN_API_VERSION  VK_MAKE_VERSION(1, 2, 0)
#define APPLICATION_NAME    "CSage testing"
#define APPLICATION_VERSION VK_MAKE_VERSION(0, 0, 1)
#define ENGINE_NAME         "CSage"
#define ENGINE_VERSION      VK_MAKE_VERSION(0, 0, 1)

#if DEBUG_LEVEL > 0
    #ifdef _WIN32
        #define VK_LAYERC 2
        #define VK_LAYERS (char const* const[]){ "VK_LAYER_KHRONOS_validation", \
                                                 "VK_LAYER_LUNARG_monitor" }
    #else
        #define VK_LAYERC 3
        #define VK_LAYERS (char const* const[]){ "VK_LAYER_KHRONOS_validation", \
                                                 "VK_LAYER_LUNARG_monitor",     \
                                                 "VK_LAYER_RENDERDOC_Capture", }
    #endif
#else
    #define VK_LAYERC 0
    #define VK_LAYERS (char const* const[]){ }
#endif

#define VK_GET_EXT(var, ext) \
	PFN_##ext var = (PFN_##ext)vkGetInstanceProcAddr(instance, #ext)
#define STRING_DEVICE_TYPE(x)                                          \
	((x) == VK_PHYSICAL_DEVICE_TYPE_OTHER         ? "Other"         : \
	 (x) == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU? "Integrated gpu": \
	 (x) == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU  ? "Discrete gpu"  : \
	 (x) == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU   ? "Virtual gpu"   : \
	 (x) == VK_PHYSICAL_DEVICE_TYPE_CPU           ? "CPU"           : \
	 "Unknown")
#define STRING_QUEUE_BIT(x)                                 \
	((x) == VK_QUEUE_GRAPHICS_BIT      ? "Graphics"      : \
	 (x) == VK_QUEUE_COMPUTE_BIT       ? "Compute"       : \
	 (x) == VK_QUEUE_TRANSFER_BIT      ? "Transfer"      : \
	 (x) == VK_QUEUE_SPARSE_BINDING_BIT? "Sparse Binding": \
	 (x) == VK_QUEUE_PROTECTED_BIT     ? "Protected"     : \
	 (x) == 0x00000007? "Transfer, Compute, Graphics"    : \
	 (x) == 0x0000000F? "Generic (unprotected)"          : \
	 (x) == 0x0000001F? "Generic (protected)"            : \
	 "Other combination")
#define STRING_PRESENT_MODE(x)                                                       \
	((x) == VK_PRESENT_MODE_IMMEDIATE_KHR                ? "Immediate"            : \
	 (x) == VK_PRESENT_MODE_MAILBOX_KHR                  ? "Mailbox"              : \
	 (x) == VK_PRESENT_MODE_FIFO_KHR                     ? "FIFO"                 : \
	 (x) == VK_PRESENT_MODE_FIFO_RELAXED_KHR             ? "FIFO relaxed"         : \
	 (x) == VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR    ? "Shared demand refresh": \
	 (x) == VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR? "Shared cont. refresh" : \
	 "Unknown")
#define STRING_DEBUG_REPORT(x)                                                   \
	((x) == VK_DEBUG_REPORT_INFORMATION_BIT_EXT        ? "Information"        : \
	 (x) == VK_DEBUG_REPORT_WARNING_BIT_EXT            ? "Warning"            : \
	 (x) == VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT? "Performance Warning": \
	 (x) == VK_DEBUG_REPORT_ERROR_BIT_EXT              ? "Error"              : \
	 (x) == VK_DEBUG_REPORT_DEBUG_BIT_EXT              ? "Debug"              : \
	 "Unknown")
#define STRING_DEBUG_REPORT_OBJECT(x)                                                                                \
    ((x) == VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT                       ? "Unknown"                       : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT                      ? "Instance"                      : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT               ? "Physical Device"               : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT                        ? "Device"                        : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT                         ? "Queue"                         : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT                     ? "Semaphore"                     : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT                ? "Command Buffer"                : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT                         ? "Fence"                         : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT                 ? "Device Memory"                 : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT                        ? "Buffer1"                       : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT                         ? "Image"                         : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT                         ? "Event"                         : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT                    ? "Query Pool"                    : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT                   ? "Buffer View"                   : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT                    ? "Image View"                    : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT                 ? "Shader Module"                 : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT                ? "Pipeline Cache"                : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT               ? "Pipeline Layout"               : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT                   ? "Render Pass"                   : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT                      ? "Pipeline"                      : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT         ? "Descriptor Set Layout"         : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT                       ? "Sampler"                       : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT               ? "Descriptor Pool"               : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT                ? "Descriptor Set"                : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT                   ? "Framebuffer"                   : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT                  ? "Command Pool"                  : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT                   ? "Surface KHR"                   : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT                 ? "Swapchain KHR"                 : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT     ? "Debug Report Callback EXT"     : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT                   ? "Display KHR"                   : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT              ? "Display Mode KHR"              : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT          ? "Validation Cache EXT"          : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT      ? "Sampler YCBBCR Conversion"     : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT    ? "Descriptor Update Template"    : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT    ? "Acceleration Structure KHR"    : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT     ? "Acceleration Structure NV"     : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_EXT                  ? "Debug Report"                  : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT              ? "Validation Cache"              : \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR_EXT? "Descriptor Update Template KHR": \
     (x) == VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_KHR_EXT  ? "Sampler YCBCR Conversion KHR"  : \
     "Unknown")
#define STRING_IMAGE_LAYOUT(x)                                                                                                                \
    ((x) == VK_IMAGE_LAYOUT_UNDEFINED                                     ? "VK_IMAGE_LAYOUT_UNDEFINED"                                     : \
     (x) == VK_IMAGE_LAYOUT_GENERAL                                       ? "VK_IMAGE_LAYOUT_GENERAL"                                       : \
     (x) == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL                      ? "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL"                      : \
     (x) == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL              ? "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL"              : \
     (x) == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL               ? "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL"               : \
     (x) == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL                      ? "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL"                      : \
     (x) == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL                          ? "VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL"                          : \
     (x) == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL                          ? "VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL"                          : \
     (x) == VK_IMAGE_LAYOUT_PREINITIALIZED                                ? "VK_IMAGE_LAYOUT_PREINITIALIZED"                                : \
     (x) == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL    ? "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL"    : \
     (x) == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL    ? "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL"    : \
     (x) == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL                      ? "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL"                      : \
     (x) == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL                       ? "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL"                       : \
     (x) == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL                    ? "VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL"                    : \
     (x) == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL                     ? "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL"                     : \
     (x) == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL                             ? "VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL"                             : \
     (x) == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL                            ? "VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL"                            : \
     (x) == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR                               ? "VK_IMAGE_LAYOUT_PRESENT_SRC_KHR"                               : \
     (x) == VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR                            ? "VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR"                            : \
     (x) == VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT              ? "VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT"              : \
     (x) == VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR  ? "VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR"  : \
     (x) == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR? "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR": \
     (x) == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR? "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR": \
     (x) == VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV                       ? "VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV"                       : \
     (x) == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR                  ? "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR"                  : \
     (x) == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR                   ? "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR"                   : \
     (x) == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR                ? "VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR"                : \
     (x) == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR                 ? "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR"                 : \
     (x) == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR                         ? "VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR"                         : \
     (x) == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR                        ? "VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR"                        : \
     "Unknown")
         
void vulkan_init(SDL_Window* window);
VkShaderModule vulkan_new_shader(const char* restrict file);
void vulkan_free();

extern VkAllocationCallbacks* alloccb;
extern VkInstance   instance;
extern VkSurfaceKHR surface;

#endif
