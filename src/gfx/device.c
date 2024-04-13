#include <vulkan/vulkan.h>

#include "vulkan.h"
#include "swapchain.h"
#include "device.h"

VkPhysicalDevice physical_gpu;
VkDevice         logical_gpu;

VkQueue graphicsq;
VkQueue presentq;
VkQueue transferq;

VkCommandPool             transfer_cmd_pool;
VkCommandPool             graphics_cmd_pool;
struct QueueFamilyIndices qinds;
struct DeviceLimits       gpu_properties;

static int  rate_device(VkPhysicalDevice dev, VkSurfaceKHR surf);
static bool supports_extension(VkPhysicalDevice dev, char* ext);
static void get_device_properties(VkPhysicalDevice dev);
static void debug_physical(VkPhysicalDevice dev);
static void set_queue_indices(VkPhysicalDevice dev, VkSurfaceKHR surf);

void device_init_physical(VkInstance inst, VkSurfaceKHR surf)
{
	uint devc;
	vkEnumeratePhysicalDevices(inst, &devc, NULL);
	if (devc == 0)
		ERROR("[VK] Failed to find a physical device with vulkan support");
	else
		INFO(TERM_DARK_GREEN "[VK] %u devices found", devc);

	VkPhysicalDevice devs[devc];
	int rating;
	int max_rating = INT_MIN;
	vkEnumeratePhysicalDevices(inst, &devc, devs);
	/* TODO: add feature available flags (e.g., anisotropic filtering) */
	for (uint i = 0; i < devc; i++) {
		rating = rate_device(devs[i], surf);
		if (rating > max_rating) {
			max_rating   = rating;
			physical_gpu = devs[i];
		}
	}

	if (!physical_gpu)
		ERROR("[VK] No suitable physical device found");
	else
		INFO(TERM_DARK_GREEN "[VK] Created physical device (rating: %d)", max_rating);
}

void device_init_logical(VkSurfaceKHR surf)
{
	// TODO: this probably needs to be generalized later
	set_queue_indices(physical_gpu, surf);
	VkDeviceQueueCreateInfo devqis[] = {
		{ .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		  .flags = 0,
		  .queueFamilyIndex = qinds.graphics,
		  .pQueuePriorities = (float[]){ 1.0f },
		  .queueCount       = 1, },
		{ .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		  .flags = 0,
		  .queueFamilyIndex = qinds.transfer,
		  .pQueuePriorities = (float[]){ 1.0f, 1.0f },
		  .queueCount       = 2, },
	};

	VkPhysicalDeviceVulkan12Features vk12_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.runtimeDescriptorArray  = true,
		.shaderInt8              = true,
		.storageBuffer8BitAccess = true,
	};
	VkPhysicalDevice16BitStorageFeatures int16_feature = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES,
		.storageBuffer16BitAccess = true,
		.pNext                    = &vk12_features,
	};

	VkDeviceCreateInfo devi = {
		.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.flags                = 0,
		.queueCreateInfoCount = ARRAY_SIZE(devqis),
		.pQueueCreateInfos    = devqis,
		.pEnabledFeatures = &(VkPhysicalDeviceFeatures){
			// .geometryShader     = true,
			// .tessellationShader = true,
			.wideLines          = true,
			.largePoints        = true,
			.shaderInt16        = true,
		},
		.pNext = &(VkPhysicalDeviceShaderDrawParametersFeatures){
			.sType                = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES,
			.shaderDrawParameters = 1,
			.pNext                = &int16_feature,
		},
		.enabledLayerCount       = VULKAN_LAYER_COUNT,
		.ppEnabledLayerNames     = VULKAN_LAYERS,
		.enabledExtensionCount   = 1,
		.ppEnabledExtensionNames = (const char*[]){ "VK_KHR_swapchain", },
	};
	if ((vk_err = vkCreateDevice(physical_gpu, &devi, NULL, &logical_gpu)))
		ERROR("[VK] Failed to create logical device: \n\t\"%s\"", STRING_OF_VK_RESULT(vk_err));
	else
		INFO(TERM_DARK_GREEN "[VK] Created logical device");

	vkGetDeviceQueue(logical_gpu, qinds.graphics, 0, &graphicsq);
	vkGetDeviceQueue(logical_gpu, qinds.present , 0, &presentq);
	vkGetDeviceQueue(logical_gpu, qinds.transfer, 1, &transferq);
}

int device_find_memory_index(uint type, VkMemoryPropertyFlagBits prop)
{
	VkPhysicalDeviceMemoryProperties memprop;
	vkGetPhysicalDeviceMemoryProperties(physical_gpu, &memprop);
	for (int i = 0; i < (int)memprop.memoryTypeCount; i++)
		if (type & (1 << i) && (memprop.memoryTypes[i].propertyFlags & prop) == prop)
			return i;

	ERROR("[VK] Failed to find suitable memory type");
	return INT_MAX;
}

void device_free()
{
	INFO(TERM_DARK_GREEN "[VK] Destroying device...");
	vkDestroyDevice(logical_gpu, NULL);
}

/* -------------------------------------------------------------------- */

static int rate_device(VkPhysicalDevice dev, VkSurfaceKHR surf)
{
	get_device_properties(dev);

	// TODO: replace with data from `get_device_limits()`
	int rating = 0;
	VkPhysicalDeviceProperties devp;
	VkPhysicalDeviceFeatures   devf;
	vkGetPhysicalDeviceProperties(dev, &devp);
	vkGetPhysicalDeviceFeatures(dev, &devf);

	char* devext[1] = { "VK_KHR_swapchain" };
	for (int i = 0; i < 1; i++)
		if (!supports_extension(dev, devext[i]))
			rating = INT_MIN;

	swapchain_set(dev, surf);
	if (swapchain_details.fmtc == 0 || swapchain_details.modec == 0)
		rating = INT_MIN;

	set_queue_indices(dev, surf);
	if (qinds.graphics == -1)
		return INT_MIN;
	if (devp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		rating += 50;
	else if (devp.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
		rating += 5;
	else if (devp.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		rating += 10;

	debug_physical(dev);
	INFO(TERM_DARK_GREEN "\tRated: %d\n", rating);
	return rating;
}

static bool supports_extension(VkPhysicalDevice dev, char* ext)
{
	uint32 extc;
	vkEnumerateDeviceExtensionProperties(dev, NULL, &extc, NULL);
	VkExtensionProperties exts[extc];
	vkEnumerateDeviceExtensionProperties(dev, NULL, &extc, exts);
	for (uint i = 0; i < extc; i++)
		if (!strcmp(exts[i].extensionName, ext))
			return true;
	return false;
}

static void set_queue_indices(VkPhysicalDevice dev, VkSurfaceKHR surf)
{
	uint qfamilyc;
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &qfamilyc, NULL);
	VkQueueFamilyProperties qfamilies[qfamilyc];
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &qfamilyc, qfamilies);

	uint pres;
	for (int i = 0; i < (int)qfamilyc; i++) {
		if (qfamilies[i].queueCount > 0) {
			vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surf, &pres);
			if (pres && !qinds.present)
				qinds.present = i;

			if (qfamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && !qinds.graphics)
				qinds.graphics = i;
			else if (qfamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT && !qinds.transfer)
				qinds.transfer = i;
		}
	}
}

static void get_device_properties(VkPhysicalDevice dev)
{
	VkPhysicalDeviceProperties devp;
	VkPhysicalDeviceMemoryProperties dev_memp;
	vkGetPhysicalDeviceProperties(dev, &devp);
	vkGetPhysicalDeviceMemoryProperties(dev, &dev_memp);

	VkSampleCountFlags count = devp.limits.framebufferColorSampleCounts & devp.limits.framebufferDepthSampleCounts;
	VkSampleCountFlagBits max_samples  = (count & VK_SAMPLE_COUNT_64_BIT? VK_SAMPLE_COUNT_64_BIT:
	                                      count & VK_SAMPLE_COUNT_32_BIT? VK_SAMPLE_COUNT_32_BIT:
	                                      count & VK_SAMPLE_COUNT_16_BIT? VK_SAMPLE_COUNT_16_BIT:
	                                      count & VK_SAMPLE_COUNT_8_BIT ? VK_SAMPLE_COUNT_8_BIT :
	                                      count & VK_SAMPLE_COUNT_4_BIT ? VK_SAMPLE_COUNT_4_BIT :
	                                      count & VK_SAMPLE_COUNT_2_BIT ? VK_SAMPLE_COUNT_2_BIT :
	                                                                      VK_SAMPLE_COUNT_1_BIT);

	gpu_properties = (struct DeviceLimits){
		.max_samples = max_samples,
	};

	for (int i = 0; i < (int)dev_memp.memoryTypeCount; i++)
		if (dev_memp.memoryTypes[i].propertyFlags == VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
			gpu_properties.lazy_mem = true;

	INFO(TERM_DARK_GREEN "[VK] GPU has the following support:");
	INFO(TERM_DARK_GREEN "\tMSAA samples     -> %d", max_samples);
	INFO(TERM_DARK_GREEN "\tLazy memory      -> %s", STR_TF(gpu_properties.lazy_mem));
	INFO(TERM_DARK_GREEN "\tPoint size range -> %1.2f - %1.2f", devp.limits.pointSizeRange[0], devp.limits.pointSizeRange[1]);
	INFO(TERM_DARK_GREEN "\tLine width range -> %1.2f - %1.2f", devp.limits.lineWidthRange[0], devp.limits.lineWidthRange[1]);
}

static void debug_physical(VkPhysicalDevice dev)
{
	uint major, minor, patch;
	VkPhysicalDeviceProperties devp;
	VkPhysicalDeviceFeatures   devf;
	vkGetPhysicalDeviceProperties(dev, &devp);
	vkGetPhysicalDeviceFeatures(dev, &devf);

	INFO(TERM_DARK_GREEN "[VK] Physical device: %s (%u)", devp.deviceName, devp.vendorID);
	INFO(TERM_DARK_GREEN "\tDevice type                       -> %s (%u)", STRING_OF_DEVICE_TYPE(devp.deviceType), devp.deviceID);
	patch = VK_VERSION_PATCH(devp.apiVersion);
	major = VK_VERSION_MAJOR(devp.apiVersion);
	minor = VK_VERSION_MINOR(devp.apiVersion);
	INFO(TERM_DARK_GREEN "\tAPI version                       -> %u.%u.%u", major, minor, patch);
	major = VK_VERSION_MAJOR(devp.driverVersion);
	minor = VK_VERSION_MINOR(devp.driverVersion);
	patch = VK_VERSION_PATCH(devp.driverVersion);
	INFO(TERM_DARK_GREEN "\tDriver version                    -> %u.%u.%u", major, minor, patch);
	INFO(TERM_DARK_GREEN "\tGeometry shader                   -> %s", STR_YN(devf.geometryShader));
	INFO(TERM_DARK_GREEN "\tTessellation shader               -> %s", STR_YN(devf.tessellationShader));
	INFO(TERM_DARK_GREEN "\tSampler anisotropy                -> %s", STR_YN(devf.samplerAnisotropy));
	INFO(TERM_DARK_GREEN "\tMSAA sample count                 -> %d", gpu_properties.max_samples);
	INFO(TERM_DARK_GREEN "\tMax image dimensions (1/2/3)      -> %u/%u/%u", devp.limits.maxImageDimension1D,
	                                                            devp.limits.maxImageDimension2D,
	                                                            devp.limits.maxImageDimension3D);
	INFO(TERM_DARK_GREEN "\tMax sampler allocations           -> %u", devp.limits.maxSamplerAllocationCount);
	INFO(TERM_DARK_GREEN "\tMax memory allocations            -> %u", devp.limits.maxMemoryAllocationCount);
	INFO(TERM_DARK_GREEN "\tMax uniform buffer range          -> %u (%uMB)", devp.limits.maxUniformBufferRange,
	                                                             devp.limits.maxUniformBufferRange/8/1024/1024);
	INFO(TERM_DARK_GREEN "\tMax storage buffer range          -> %u (%uMB)", devp.limits.maxStorageBufferRange,
	                                                             devp.limits.maxStorageBufferRange/8/1024/1024);
	INFO(TERM_DARK_GREEN "\tMax push constants size           -> %uB", devp.limits.maxPushConstantsSize);
	INFO(TERM_DARK_GREEN "\tMax bound descriptor sets         -> %u", devp.limits.maxBoundDescriptorSets);
	INFO(TERM_DARK_GREEN "\tMax vertex input attributes       -> %u", devp.limits.maxVertexInputAttributes);
	INFO(TERM_DARK_GREEN "\tMax vertex input bindings         -> %u", devp.limits.maxVertexInputBindings);
	INFO(TERM_DARK_GREEN "\tMax vertex input attribute offset -> %u", devp.limits.maxVertexInputAttributeOffset);
	INFO(TERM_DARK_GREEN "\tMax vertex input binding stride   -> %u", devp.limits.maxVertexInputBindingStride);
	INFO(TERM_DARK_GREEN "\tMax vertex output components      -> %u", devp.limits.maxVertexOutputComponents);
	INFO(TERM_DARK_GREEN "\tMipmap precision bits             -> %u", devp.limits.mipmapPrecisionBits);
	INFO(TERM_DARK_GREEN "\tMax draw indexed index value      -> %u", devp.limits.maxDrawIndexedIndexValue);
	INFO(TERM_DARK_GREEN "\tMax draw indirect count           -> %u", devp.limits.maxDrawIndirectCount);
	INFO(TERM_DARK_GREEN "\tMax sampler LoD bias              -> %f", devp.limits.maxSamplerLodBias);
	INFO(TERM_DARK_GREEN "\tMax sampler anisotropy            -> %f", devp.limits.maxSamplerAnisotropy);
	INFO(TERM_DARK_GREEN "\tMax clip distances                -> %u", devp.limits.maxClipDistances);
	INFO(TERM_DARK_GREEN "\tMax cull distances                -> %u", devp.limits.maxCullDistances);
	INFO(TERM_DARK_GREEN "\tMax combined clip/cull distances  -> %u", devp.limits.maxCombinedClipAndCullDistances);

	uint qc;
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &qc, NULL);
	VkQueueFamilyProperties qs[qc];
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &qc, qs);
	INFO(TERM_DARK_GREEN "\t%u device queue families:", qc);
	for (uint i = 0; i < qc; i++)
		INFO(TERM_DARK_GREEN "\t    %2d of 0x%.8X -> %s", qs[i].queueCount,
		      qs[i].queueFlags, STRING_OF_QUEUE_BIT(qs[i].queueFlags));

	INFO(TERM_DARK_GREEN "\t%u colour formats available", swapchain_details.fmtc);

	INFO(TERM_DARK_GREEN "\tSurface Capabilities:");
	INFO(TERM_DARK_GREEN "\t    Image count  -> %u..%u", swapchain_details.abilities.minImageCount,
	                                         swapchain_details.abilities.maxImageCount);
	INFO(TERM_DARK_GREEN "\t    Extent       -> %ux%u (%ux%u..%ux%u)",
	      swapchain_details.abilities.currentExtent.width , swapchain_details.abilities.currentExtent.height,
	      swapchain_details.abilities.minImageExtent.width, swapchain_details.abilities.minImageExtent.height,
	      swapchain_details.abilities.maxImageExtent.width, swapchain_details.abilities.maxImageExtent.height);
	INFO(TERM_DARK_GREEN "\t    Array layers -> %u", swapchain_details.abilities.maxImageArrayLayers);

	INFO(TERM_DARK_GREEN "\t%u present modes:", swapchain_details.modec);
	for (uint i = 0; i < swapchain_details.modec; i++)
		INFO(TERM_DARK_GREEN "\t    %s", STRING_OF_PRESENT_MODE(swapchain_details.modes[i]));
}

