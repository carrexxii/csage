#ifndef GFX_VULKAN_H
#define GFX_VULKAN_H

#include <vulkan/vulkan.h>
#include "SDL3/SDL.h"

#include "common.h"
#include "device.h"
#include "pipeline.h"
#include "buffers.h"

#define VULKAN_API_VERSION  VK_MAKE_VERSION(1, 3, 0)
#define APPLICATION_NAME    "CSage testing"
#define APPLICATION_VERSION VK_MAKE_VERSION(0, 0, 1)
#define ENGINE_NAME         "CSage"
#define ENGINE_VERSION      VK_MAKE_VERSION(0, 0, 1)

#if DEBUG_LEVEL > 0
	#ifdef _WIN32
		#define VULKAN_LAYER_COUNT 2
		#define VULKAN_LAYERS (char const* const[]){ "VK_LAYER_KHRONOS_validation", \
		                                             "VK_LAYER_LUNARG_monitor" }
	#else
		#define VULKAN_LAYER_COUNT 1
		#define VULKAN_LAYERS (char const* const[]){ "VK_LAYER_KHRONOS_validation" }
	#endif
#else
	#define VULKAN_LAYER_COUNT 0
	#define VULKAN_LAYERS (char const* const[]){ }
#endif

#define VK_GET_EXT(var, ext) \
	PFN_##ext var = (PFN_##ext)vkGetInstanceProcAddr(instance, #ext)
#define STRING_OF_DEVICE_TYPE(x)                                      \
	((x) == VK_PHYSICAL_DEVICE_TYPE_OTHER         ? "Other"         : \
	 (x) == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU? "Integrated gpu": \
	 (x) == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU  ? "Discrete gpu"  : \
	 (x) == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU   ? "Virtual gpu"   : \
	 (x) == VK_PHYSICAL_DEVICE_TYPE_CPU           ? "CPU"           : \
	 "Unknown")
#define STRING_OF_QUEUE_BIT(x)                             \
	((x) == VK_QUEUE_GRAPHICS_BIT      ? "Graphics"      : \
	 (x) == VK_QUEUE_COMPUTE_BIT       ? "Compute"       : \
	 (x) == VK_QUEUE_TRANSFER_BIT      ? "Transfer"      : \
	 (x) == VK_QUEUE_SPARSE_BINDING_BIT? "Sparse Binding": \
	 (x) == VK_QUEUE_PROTECTED_BIT     ? "Protected"     : \
	 (x) == 0x00000006? "Transfer, Compute"              : \
	 (x) == 0x00000007? "Transfer, Compute, Graphics"    : \
	 (x) == 0x0000000F? "Generic (unprotected)"          : \
	 (x) == 0x0000001F? "Generic (protected)"            : \
	 "Other combination")
#define STRING_OF_PRESENT_MODE(x)                                                   \
	((x) == VK_PRESENT_MODE_IMMEDIATE_KHR                ? "Immediate"            : \
	 (x) == VK_PRESENT_MODE_MAILBOX_KHR                  ? "Mailbox"              : \
	 (x) == VK_PRESENT_MODE_FIFO_KHR                     ? "FIFO"                 : \
	 (x) == VK_PRESENT_MODE_FIFO_RELAXED_KHR             ? "FIFO relaxed"         : \
	 (x) == VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR    ? "Shared demand refresh": \
	 (x) == VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR? "Shared cont. refresh" : \
	 "Unknown")
#define STRING_OF_DEBUG_REPORT(x)                                               \
	((x) == VK_DEBUG_REPORT_INFORMATION_BIT_EXT        ? "Information"        : \
	 (x) == VK_DEBUG_REPORT_WARNING_BIT_EXT            ? "Warning"            : \
	 (x) == VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT? "Performance Warning": \
	 (x) == VK_DEBUG_REPORT_ERROR_BIT_EXT              ? "Error"              : \
	 (x) == VK_DEBUG_REPORT_DEBUG_BIT_EXT              ? "Debug"              : \
	 "Unknown")
#define STRING_OF_DEBUG_REPORT_OBJECT(x)                                                                      \
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

#define STRING_OF_VK_RESULT(x)                                                                                              \
	((x) == VK_SUCCESS                                           ? "VK_SUCCESS"                                           : \
     (x) == VK_NOT_READY                                         ? "VK_NOT_READY"                                         : \
     (x) == VK_TIMEOUT                                           ? "VK_TIMEOUT"                                           : \
     (x) == VK_EVENT_SET                                         ? "VK_EVENT_SET"                                         : \
     (x) == VK_EVENT_RESET                                       ? "VK_EVENT_RESET"                                       : \
     (x) == VK_INCOMPLETE                                        ? "VK_INCOMPLETE"                                        : \
     (x) == VK_ERROR_OUT_OF_HOST_MEMORY                          ? "VK_ERROR_OUT_OF_HOST_MEMORY"                          : \
     (x) == VK_ERROR_OUT_OF_DEVICE_MEMORY                        ? "VK_ERROR_OUT_OF_DEVICE_MEMORY"                        : \
     (x) == VK_ERROR_INITIALIZATION_FAILED                       ? "VK_ERROR_INITIALIZATION_FAILED"                       : \
     (x) == VK_ERROR_DEVICE_LOST                                 ? "VK_ERROR_DEVICE_LOST"                                 : \
     (x) == VK_ERROR_MEMORY_MAP_FAILED                           ? "VK_ERROR_MEMORY_MAP_FAILED"                           : \
     (x) == VK_ERROR_LAYER_NOT_PRESENT                           ? "VK_ERROR_LAYER_NOT_PRESENT"                           : \
     (x) == VK_ERROR_EXTENSION_NOT_PRESENT                       ? "VK_ERROR_EXTENSION_NOT_PRESENT"                       : \
     (x) == VK_ERROR_FEATURE_NOT_PRESENT                         ? "VK_ERROR_FEATURE_NOT_PRESENT"                         : \
     (x) == VK_ERROR_INCOMPATIBLE_DRIVER                         ? "VK_ERROR_INCOMPATIBLE_DRIVER"                         : \
     (x) == VK_ERROR_TOO_MANY_OBJECTS                            ? "VK_ERROR_TOO_MANY_OBJECTS"                            : \
     (x) == VK_ERROR_FORMAT_NOT_SUPPORTED                        ? "VK_ERROR_FORMAT_NOT_SUPPORTED"                        : \
     (x) == VK_ERROR_FRAGMENTED_POOL                             ? "VK_ERROR_FRAGMENTED_POOL"                             : \
     (x) == VK_ERROR_UNKNOWN                                     ? "VK_ERROR_UNKNOWN"                                     : \
     (x) == VK_ERROR_OUT_OF_POOL_MEMORY                          ? "VK_ERROR_OUT_OF_POOL_MEMORY"                          : \
     (x) == VK_ERROR_INVALID_EXTERNAL_HANDLE                     ? "VK_ERROR_INVALID_EXTERNAL_HANDLE"                     : \
     (x) == VK_ERROR_FRAGMENTATION                               ? "VK_ERROR_FRAGMENTATION"                               : \
     (x) == VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS              ? "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS"              : \
     (x) == VK_PIPELINE_COMPILE_REQUIRED                         ? "VK_PIPELINE_COMPILE_REQUIRED"                         : \
     (x) == VK_ERROR_SURFACE_LOST_KHR                            ? "VK_ERROR_SURFACE_LOST_KHR"                            : \
     (x) == VK_ERROR_NATIVE_WINDOW_IN_USE_KHR                    ? "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR"                    : \
     (x) == VK_SUBOPTIMAL_KHR                                    ? "VK_SUBOPTIMAL_KHR"                                    : \
     (x) == VK_ERROR_OUT_OF_DATE_KHR                             ? "VK_ERROR_OUT_OF_DATE_KHR"                             : \
     (x) == VK_ERROR_INCOMPATIBLE_DISPLAY_KHR                    ? "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR"                    : \
     (x) == VK_ERROR_VALIDATION_FAILED_EXT                       ? "VK_ERROR_VALIDATION_FAILED_EXT"                       : \
     (x) == VK_ERROR_INVALID_SHADER_NV                           ? "VK_ERROR_INVALID_SHADER_NV"                           : \
     (x) == VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR               ? "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR"               : \
     (x) == VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR      ? "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR"      : \
     (x) == VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR   ? "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR"   : \
     (x) == VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR      ? "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR"      : \
     (x) == VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR       ? "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR"       : \
     (x) == VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR         ? "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR"         : \
     (x) == VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT? "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT": \
     (x) == VK_ERROR_NOT_PERMITTED_KHR                           ? "VK_ERROR_NOT_PERMITTED_KHR"                           : \
     (x) == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT         ? "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT"         : \
     (x) == VK_THREAD_IDLE_KHR                                   ? "VK_THREAD_IDLE_KHR"                                   : \
     (x) == VK_THREAD_DONE_KHR                                   ? "VK_THREAD_DONE_KHR"                                   : \
     (x) == VK_OPERATION_DEFERRED_KHR                            ? "VK_OPERATION_DEFERRED_KHR"                            : \
     (x) == VK_OPERATION_NOT_DEFERRED_KHR                        ? "VK_OPERATION_NOT_DEFERRED_KHR"                        : \
     (x) == VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR            ? "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR"            : \
     (x) == VK_ERROR_COMPRESSION_EXHAUSTED_EXT                   ? "VK_ERROR_COMPRESSION_EXHAUSTED_EXT"                   : \
     (x) == VK_INCOMPATIBLE_SHADER_BINARY_EXT                    ? "VK_INCOMPATIBLE_SHADER_BINARY_EXT"                    : \
     (x) == VK_ERROR_OUT_OF_POOL_MEMORY_KHR                      ? "VK_ERROR_OUT_OF_POOL_MEMORY_KHR"                      : \
     (x) == VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR                 ? "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR"                 : \
     (x) == VK_ERROR_FRAGMENTATION_EXT                           ? "VK_ERROR_FRAGMENTATION_EXT"                           : \
     (x) == VK_ERROR_NOT_PERMITTED_EXT                           ? "VK_ERROR_NOT_PERMITTED_EXT"                           : \
     (x) == VK_ERROR_INVALID_DEVICE_ADDRESS_EXT                  ? "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT"                  : \
     (x) == VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR          ? "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR"          : \
     (x) == VK_PIPELINE_COMPILE_REQUIRED_EXT                     ? "VK_PIPELINE_COMPILE_REQUIRED_EXT"                     : \
     (x) == VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT               ? "VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT"               : \
     (x) == VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT              ? "VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT"              : \
	"Unknown")

void           init_vulkan(SDL_Window* window);
VkShaderModule create_shader(char* restrict file);
void           vulkan_free(void);

extern VkInstance   instance;
extern VkSurfaceKHR surface;

#endif

