#include <vulkan/vulkan.h>

#include "vulkan.h"
#include "swapchain.h"
#include "device.h"

VkPhysicalDevice physicalgpu;
VkDevice         gpu;

VkQueue graphicsq;
VkQueue presentq;
VkQueue transferq;

VkCommandPool cmdpool;
struct QueueFamilyIndices qinds;

static int  rate_device(VkPhysicalDevice dev, VkSurfaceKHR surf);
static bool supports_extension(VkPhysicalDevice dev, const char* ext);
static void debug_physical(VkPhysicalDevice dev);
static void set_queue_indices(VkPhysicalDevice dev, VkSurfaceKHR surf);

void device_init_physical(VkInstance inst, VkSurfaceKHR surf)
{
	uint devc;
	vkEnumeratePhysicalDevices(inst, &devc, NULL);
	if (devc == 0)
		ERROR("[VK] Failed to find a physical device with vulkan support");
	else
		DEBUG(3, "[VK] %u devices found", devc);

	VkPhysicalDevice devs[devc];
	int rating;
	int maxrating = INT_MIN;
	vkEnumeratePhysicalDevices(inst, &devc, devs);
	/* TODO: add feature available flags (e.g., anisotropic filtering) */
	for (uint i = 0; i < devc; i++) {
		rating = rate_device(devs[i], surf);
		if (rating > maxrating) {
			maxrating   = rating;
			physicalgpu = devs[i];
		}
	}

	if (!physicalgpu)
		ERROR("[VK] No suitable physical device found");
	else
		DEBUG(3, "[VK] Created physical device (rating: %d)", maxrating);
}

void device_init_logical(VkInstance inst, VkSurfaceKHR surf)
{
	set_queue_indices(physicalgpu, surf);
	const uint devqic = 2;
	VkDeviceQueueCreateInfo devqis[devqic];
	float prio = 1.0;
	for (uint i = 0; i < devqic; i++)
		devqis[i] = (VkDeviceQueueCreateInfo){
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.flags = 0,
			.queueFamilyIndex = ((int[]){ qinds.graphics,
				                          qinds.present,
									      qinds.transfer })[i],
			.pQueuePriorities = &prio,
			.queueCount       = 1,
		};

	VkPhysicalDeviceFeatures devfeatures[] = {
		{ .geometryShader = 1, }
	};
	VkPhysicalDeviceShaderDrawParametersFeatures shaderfeatures[] = {
		{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES,
		  .shaderDrawParameters = 1, },
	};

	const uint  extc   = 1;
	const char* exts[] = { "VK_KHR_swapchain" };
	VkDeviceCreateInfo devi = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.flags = 0,
		.queueCreateInfoCount    = devqic,
		.pQueueCreateInfos       = devqis,
		.pEnabledFeatures        = devfeatures,
		.pNext                   = shaderfeatures,
		.enabledLayerCount       = VK_LAYERC,
		.ppEnabledLayerNames     = VK_LAYERS,
		.enabledExtensionCount   = extc,
		.ppEnabledExtensionNames = exts,
	};
	if (vkCreateDevice(physicalgpu, &devi, alloccb, &gpu) != VK_SUCCESS)
		ERROR("[VK] Failed to create logical device");
	else
		DEBUG(3, "[VK] Created logical device");

	vkGetDeviceQueue(gpu, qinds.present , 0, &presentq);
	vkGetDeviceQueue(gpu, qinds.graphics, 0, &graphicsq);
	vkGetDeviceQueue(gpu, qinds.transfer, 0, &transferq);
}

void device_free()
{
	DEBUG(3, "[VK] Destroying device...");
	vkDestroyDevice(gpu, alloccb);
}

static int rate_device(VkPhysicalDevice dev, VkSurfaceKHR surf)
{
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
	if (swapdetails.fmtc == 0 || swapdetails.mdc == 0)
		rating = INT_MIN;

	set_queue_indices(dev, surf);
	if (qinds.graphics == -1)
		return INT_MIN;
	if (devp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		rating += 50;
	else if (devp.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		rating += 10;

	debug_physical(dev);
	DEBUG(3, "\tRated: %d", rating);
	return rating;
}

static bool supports_extension(VkPhysicalDevice dev, const char* ext)
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
	uint qfampc;
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &qfampc, NULL);
	VkQueueFamilyProperties qfamps[qfampc];
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &qfampc, qfamps);
	for (uint i = 0; i < qfampc; i++) {
		if (qfamps[i].queueCount > 0) {
			uint pres;
			vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surf, &pres);
			if (pres)
				qinds.present = i;
			if (qfamps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				qinds.graphics = i;
			if (qfamps[i].queueFlags == VK_QUEUE_TRANSFER_BIT)
				qinds.transfer = i;
			else if (qfamps[i].queueFlags & VK_QUEUE_TRANSFER_BIT && !qinds.transfer)
				qinds.transfer = i;
		}

		if (qinds.graphics > 0 && qinds.present > 0 && qinds.transfer > 0)
			break;
	}
}

static void debug_physical(VkPhysicalDevice dev)
{
	uint major, minor, patch;
	VkPhysicalDeviceProperties devp;
	VkPhysicalDeviceFeatures   devf;
	vkGetPhysicalDeviceProperties(dev, &devp);
	vkGetPhysicalDeviceFeatures(dev, &devf);

	DEBUG(3, "[VK] Physical device: %s (%u)", devp.deviceName, devp.vendorID);
	DEBUG(3, "\tDevice type                       -> %s (%u)", STRING_DEVICE_TYPE(devp.deviceType), devp.deviceID);
	patch = VK_VERSION_PATCH(devp.apiVersion);
	major = VK_VERSION_MAJOR(devp.apiVersion);
	minor = VK_VERSION_MINOR(devp.apiVersion);
	DEBUG(3, "\tAPI version                       -> %u.%u.%u", major, minor, patch);
	major = VK_VERSION_MAJOR(devp.driverVersion);
	minor = VK_VERSION_MINOR(devp.driverVersion);
	patch = VK_VERSION_PATCH(devp.driverVersion);
	DEBUG(3, "\tDriver version                    -> %u.%u.%u", major, minor, patch);
	DEBUG(3, "\tGeometry shader                   -> %s", STRING_YN(devf.geometryShader));
	DEBUG(3, "\tTessellation shader               -> %s", STRING_YN(devf.tessellationShader));
	DEBUG(3, "\tSampler anisotropy                -> %s", STRING_YN(devf.samplerAnisotropy));
	DEBUG(3, "\tMax image dimensions (1/2/3)      -> %u/%u/%u", devp.limits.maxImageDimension1D,
	                                                            devp.limits.maxImageDimension2D,
	                                                            devp.limits.maxImageDimension3D);
	DEBUG(3, "\tMax sampler allocations           -> %u", devp.limits.maxSamplerAllocationCount);
	DEBUG(3, "\tMax memory allocations            -> %u", devp.limits.maxMemoryAllocationCount);
	DEBUG(3, "\tMax uniform buffer range          -> %u (%uMB)", devp.limits.maxUniformBufferRange,
	                                                             devp.limits.maxUniformBufferRange/8/1024/1024);
	DEBUG(3, "\tMax storage buffer range          -> %u (%uMB)", devp.limits.maxStorageBufferRange,
	                                                             devp.limits.maxStorageBufferRange/8/1024/1024);
	DEBUG(3, "\tMax push constants size           -> %uB", devp.limits.maxPushConstantsSize);
	DEBUG(3, "\tMax bound descriptor sets         -> %u", devp.limits.maxBoundDescriptorSets);
	DEBUG(3, "\tMax vertex input attributes       -> %u", devp.limits.maxVertexInputAttributes);
	DEBUG(3, "\tMax vertex input bindings         -> %u", devp.limits.maxVertexInputBindings);
	DEBUG(3, "\tMax vertex input attribute offset -> %u", devp.limits.maxVertexInputAttributeOffset);
	DEBUG(3, "\tMax vertex input binding stride   -> %u", devp.limits.maxVertexInputBindingStride);
	DEBUG(3, "\tMax vertex output components      -> %u", devp.limits.maxVertexOutputComponents);
	DEBUG(3, "\tMipmap precision bits             -> %u", devp.limits.mipmapPrecisionBits);
	DEBUG(3, "\tMax draw indexed index value      -> %u", devp.limits.maxDrawIndexedIndexValue);
	DEBUG(3, "\tMax draw indirect count           -> %u", devp.limits.maxDrawIndirectCount);
	DEBUG(3, "\tMax sampler LoD bias              -> %f", devp.limits.maxSamplerLodBias);
	DEBUG(3, "\tMax sampler anisotropy            -> %f", devp.limits.maxSamplerAnisotropy);
	DEBUG(3, "\tMax clip distances                -> %u", devp.limits.maxClipDistances);
	DEBUG(3, "\tMax cull distances                -> %u", devp.limits.maxCullDistances);
	DEBUG(3, "\tMax combined clip/cull distances  -> %u", devp.limits.maxCombinedClipAndCullDistances);

	uint qc;
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &qc, NULL);
	VkQueueFamilyProperties qs[qc];
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &qc, qs);
	DEBUG(3, "\t%u device queue families:", qc);
	for (uint i = 0; i < qc; i++)
		DEBUG(3, "\t    %2d of 0x%.8X -> %s", qs[i].queueCount,
			qs[i].queueFlags, STRING_QUEUE_BIT(qs[i].queueFlags));

	DEBUG(3, "\t%u colour formats available", swapdetails.fmtc);

	DEBUG(3, "\tSurface Capabilities:");
	DEBUG(3, "\t    Image count  -> %u..%u", swapdetails.abilities.minImageCount,
	                                         swapdetails.abilities.maxImageCount);
	DEBUG(3, "\t    Extent       -> %ux%u (%ux%u..%ux%u)",
		swapdetails.abilities.currentExtent.width , swapdetails.abilities.currentExtent.height,
		swapdetails.abilities.minImageExtent.width, swapdetails.abilities.minImageExtent.height,
		swapdetails.abilities.maxImageExtent.width, swapdetails.abilities.maxImageExtent.height);
	DEBUG(3, "\t    Array layers -> %u", swapdetails.abilities.maxImageArrayLayers);

	DEBUG(3, "\t%u present modes:", swapdetails.mdc);
	for (uint i = 0; i < swapdetails.mdc; i++)
		DEBUG(3, "\t    %s", STRING_PRESENT_MODE(swapdetails.mds[i]));
}
