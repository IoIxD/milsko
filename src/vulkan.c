/* $Id$ */
#include "Mw/TypeDefs.h"
#include <Mw/Milsko.h>

#include <X11/Xlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR 1
#include <vulkan/vulkan_win32.h>
#endif
#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR 1
#include <vulkan/vulkan_xlib.h>
#endif

#include <dlfcn.h>
#include <assert.h>
#include <stdbool.h>

#define VK_CMD(func) \
	vk_res = func; \
	if(vk_res != VK_SUCCESS) { \
		printf("VULKAN ERROR AT %s:%d: %s\n", __FILE__, __LINE__, string_VkResult(vk_res)); \
		exit(0); \
	}

bool enableValidationLayers = true;

typedef struct vulkan {
	void*			  vulkanLibrary;
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
	VkInstance		  vkInstance;
	VkSurfaceKHR		  vkSurface;

	VkPhysicalDevice vkPhysicalDevice;
	VkDevice	 vkLogicalDevice;
	VkQueue		 vkQueue;
	uint32_t	 graphics_family_idx;
} vulkan_t;

static void vulkan_instance_setup(MwWidget handle, vulkan_t* o);
static void vulkan_surface_setup(MwWidget handle, vulkan_t* o);
static void vulkan_devices_setup(MwWidget handle, vulkan_t* o);

static void create(MwWidget handle) {
	vulkan_t* o = malloc(sizeof(*o));

	vulkan_instance_setup(handle, o);
	vulkan_surface_setup(handle, o);
	vulkan_devices_setup(handle, o);

	handle->internal = o;
	MwSetDefault(handle);
}

static void destroy(MwWidget handle) {
	vulkan_t* o = (vulkan_t*)handle->internal;
	free(o);
}

static void vulkan_instance_setup(MwWidget handle, vulkan_t* o) {
	// todo: Some sort of function for being able to set the vulkan version?
	uint32_t      vulkan_version  = VK_VERSION_1_0;
	uint32_t      api_version     = VK_API_VERSION_1_0;
	uint32_t      extension_count = 0;
	uint32_t      layer_count     = 0;
	uint32_t      enabled_layer_count, enabled_extension_count = 0;
	unsigned long i = 0;

	PFN_vkEnumerateInstanceExtensionProperties
					       vk_extensionPropertiesFunc;
	PFN_vkEnumerateInstanceLayerProperties vk_layerPropertiesFunc;
	PFN_vkCreateInstance		       vk_createInstanceFunc;

	VkApplicationInfo    app_info;
	VkInstanceCreateInfo instance_create_info;

	VkExtensionProperties* ext_props;
	VkLayerProperties*     layer_props;

	const char** extensions;
	const char** layers;
	VkResult     vk_res;

	// TODO: support for whatever win32's equivalants to dlopen/dlsym are
	o->vulkanLibrary	 = dlopen("libvulkan.so", RTLD_LAZY | RTLD_GLOBAL);
	o->vkGetInstanceProcAddr = dlsym(o->vulkanLibrary, "vkGetInstanceProcAddr");
	assert(o->vkGetInstanceProcAddr);

	// Load in any other function pointers we need.
	vk_extensionPropertiesFunc = dlsym(o->vulkanLibrary, "vkEnumerateInstanceExtensionProperties");
	assert(vk_extensionPropertiesFunc);
	vk_layerPropertiesFunc = dlsym(o->vulkanLibrary, "vkEnumerateInstanceLayerProperties");
	assert(vk_layerPropertiesFunc);
	vk_createInstanceFunc = dlsym(o->vulkanLibrary, "vkCreateInstance");
	assert(vk_createInstanceFunc);

	// get all the supported extensions under Vulkan so we can pass them to the vulkan creation stuff.
	// TODO: some function for setting which functions to enable or disable.

	// passing null gives us all the extensions provided by the current vulkan implementation
	VK_CMD(vk_extensionPropertiesFunc(NULL, &extension_count, NULL));
	ext_props = malloc(sizeof(VkExtensionProperties) * extension_count);
	VK_CMD(vk_extensionPropertiesFunc(NULL, &extension_count, ext_props));
	extensions = malloc(sizeof(const char*) * (extension_count + 2));

	for(i = 0; i < extension_count; i++) {
		extensions[i] = ext_props[i].extensionName;
		enabled_extension_count++;
	}
	extensions[i + 1] = NULL;

	app_info = (VkApplicationInfo){
	    .sType		= VK_STRUCTURE_TYPE_APPLICATION_INFO,
	    .pNext		= NULL,
	    .pApplicationName	= "",
	    .applicationVersion = vulkan_version,
	    .pEngineName	= "",
	    .engineVersion	= vulkan_version,
	    .apiVersion		= api_version,
	};

	VK_CMD(vk_layerPropertiesFunc(&layer_count, NULL));
	layer_props = malloc(sizeof(VkLayerProperties) * layer_count);
	VK_CMD(vk_layerPropertiesFunc(&layer_count, layer_props));
	layers = malloc(256 * (layer_count + 2));
	for(i = 0; i < layer_count; i++) {
		if(enableValidationLayers) {
			if(strcmp(layer_props[i].layerName, "VK_LAYER_KHRONOS_validation") == 0) {
				printf("layer: %s\n", layer_props[i].layerName);
				memset(&layers[i], 0, 255);
				memcpy(&layers[i], layer_props[i].layerName, 254);
				enabled_layer_count++;
				break;
			} else {
				continue;
			}
		}
	}
	layers[i + 1] = NULL;

	instance_create_info = (VkInstanceCreateInfo){
	    .sType		     = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
	    .flags		     = 0,
	    .pApplicationInfo	     = &app_info,
	    .enabledExtensionCount   = enabled_extension_count,
	    .enabledLayerCount	     = 0,
	    .ppEnabledExtensionNames = extensions,
	    .ppEnabledLayerNames     = layers,
	};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	VK_CMD(vk_createInstanceFunc(&instance_create_info, NULL, &o->vkInstance));

	free(extensions);
	free(layers);
}

static void vulkan_surface_setup(MwWidget handle, vulkan_t* o) {
	int vk_res;
#ifdef _WIN32
	PFN_vkCreateWin32SurfaceKHR surfaceCreationFunction =
	    (PFN_vkCreateWin32SurfaceKHR)o->vkGetInstanceProcAddr(o->vkInstance, "vkCreateWin32SurfaceKHR");
	assert(surfaceCreationFunction);

	VkWin32SurfaceCreateInfoKHR createInfo = {
	    .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
	    .pNext     = NULL,
	    .flags     = 0,
	    .hinstance = handle->lowlevel->hInstance,
	    .hwnd      = handle->lowlevel->hWnd,
	};

	VK_CMD(surfaceCreationFunction(o->vkInstance, &createInfo, NULL,
				       &o->vkSurface));
#endif
#ifdef __linux__
	PFN_vkCreateXlibSurfaceKHR surfaceCreationFunction =
	    (PFN_vkCreateXlibSurfaceKHR)o->vkGetInstanceProcAddr(o->vkInstance, "vkCreateXlibSurfaceKHR");
	assert(surfaceCreationFunction);

	VkXlibSurfaceCreateInfoKHR createInfo = {
	    .sType  = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
	    .pNext  = NULL,
	    .flags  = 0,
	    .dpy    = handle->lowlevel->display,
	    .window = handle->lowlevel->window,
	};
	VK_CMD(surfaceCreationFunction(o->vkInstance, &createInfo, NULL, &o->vkSurface));
#endif
}

static void vulkan_devices_setup(MwWidget handle, vulkan_t* o) {
	int			 vk_res;
	unsigned long		 i, n;
	uint32_t		 deviceCount;
	VkPhysicalDevice*	 devices;
	uint32_t		 familyPropCount;
	VkQueueFamilyProperties* family_props;
	float			 queuePriority = 1.0f;
	VkDeviceQueueCreateInfo	 queueCreateInfo;
	uint32_t		 uniqueQueueFamilies[1];
	VkDeviceCreateInfo	 createInfo;

	PFN_vkEnumeratePhysicalDevices physicalDevicesFunc =
	    (PFN_vkEnumeratePhysicalDevices)o->vkGetInstanceProcAddr(o->vkInstance, "vkEnumeratePhysicalDevices");
	assert(physicalDevicesFunc);

	PFN_vkGetPhysicalDeviceQueueFamilyProperties queueFamilyPropertiesFunc =
	    (PFN_vkGetPhysicalDeviceQueueFamilyProperties)o->vkGetInstanceProcAddr(o->vkInstance, "vkGetPhysicalDeviceQueueFamilyProperties");
	assert(queueFamilyPropertiesFunc);

	PFN_vkCreateDevice createDeviceFunc = (PFN_vkCreateDevice)o->vkGetInstanceProcAddr(o->vkInstance, "vkCreateDevice");
	assert(createDeviceFunc);

	PFN_vkGetDeviceQueue getDeviceQueueFunc = (PFN_vkGetDeviceQueue)o->vkGetInstanceProcAddr(o->vkInstance, "vkGetDeviceQueue");
	assert(createDeviceFunc);

	// create the physical device
	VK_CMD(physicalDevicesFunc(o->vkInstance, &deviceCount, NULL));
	devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
	VK_CMD(physicalDevicesFunc(o->vkInstance, &deviceCount, devices));

	for(i = 0; i < deviceCount; i++) {
		bool has_idx = false;

		queueFamilyPropertiesFunc(devices[i], &familyPropCount, NULL);
		family_props	       = malloc(sizeof(VkQueueFamilyProperties) * familyPropCount);
		o->graphics_family_idx = 0;
		for(n = 0; n < familyPropCount; n++) {
			if((family_props[n].queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) {
				has_idx = true;
				break;
			};
			o->graphics_family_idx++;
		}
		free(family_props);
		if(has_idx) {
			o->vkPhysicalDevice = devices[i];
			break;
		}
	}
	uniqueQueueFamilies[0] = o->graphics_family_idx;

	// create the logical device
	queueCreateInfo = (VkDeviceQueueCreateInfo){
	    queueCreateInfo.sType	     = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
	    queueCreateInfo.pNext	     = NULL,
	    queueCreateInfo.flags	     = 0,
	    queueCreateInfo.queueFamilyIndex = o->graphics_family_idx,
	    queueCreateInfo.queueCount	     = 1,
	    queueCreateInfo.pQueuePriorities = &queuePriority,
	};

	createInfo = (VkDeviceCreateInfo){
	    .sType		     = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	    .pNext		     = NULL,
	    .flags		     = 0,
	    .queueCreateInfoCount    = 1,
	    .pQueueCreateInfos	     = &queueCreateInfo,
	    .pEnabledFeatures	     = NULL,
	    .enabledExtensionCount   = 0,
	    .ppEnabledExtensionNames = NULL,
	    .enabledLayerCount	     = 0,
	};

	VK_CMD(createDeviceFunc(o->vkPhysicalDevice, &createInfo, NULL, &o->vkLogicalDevice) != VK_SUCCESS);

	getDeviceQueueFunc(o->vkLogicalDevice, o->graphics_family_idx, 0, &o->vkQueue);
	// free(devices);
}

PFN_vkGetInstanceProcAddr MwVulkanGetInstanceProcAddr(MwWidget handle) {
	return ((vulkan_t*)handle->internal)->vkGetInstanceProcAddr;
};
VkInstance MwVulkanGetInstance(MwWidget handle) {
	return ((vulkan_t*)handle->internal)->vkInstance;
};
VkSurfaceKHR MwVulkanGetSurface(MwWidget handle) {
	return ((vulkan_t*)handle->internal)->vkSurface;
};
VkPhysicalDevice MwVulkanGetPhysicalDevice(MwWidget handle) {
	return ((vulkan_t*)handle->internal)->vkPhysicalDevice;
};
VkDevice MwVulkanGetLogicalDevice(MwWidget handle) {
	return ((vulkan_t*)handle->internal)->vkLogicalDevice;
};
VkQueue MwVulkanGetQueue(MwWidget handle) {
	return ((vulkan_t*)handle->internal)->vkQueue;
};

int MwVulkanGetGraphicsQueueIndex(MwWidget handle) {
	return ((vulkan_t*)handle->internal)->graphics_family_idx;
}

MwClassRec MwVulkanClassRec = {
    create,  /* create */
    destroy, /* destroy */
    NULL,    /* draw */
    NULL     /* click */
};
MwClass MwVulkanClass = &MwVulkanClassRec;
