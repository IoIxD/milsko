/* $Id$ */
#ifndef __MW_VULKAN_H__
#define __MW_VULKAN_H__

#if !defined(_WIN32) && !defined(__linux__)
#error Vulkan is unsupported on the requested platform.
#endif

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <Mw/MachDep.h>
#include <Mw/TypeDefs.h>

#ifdef __cplusplus
extern "C" {
#endif

MWDECL MwClass MwVulkanClass;

MWDECL PFN_vkGetInstanceProcAddr MwVulkanGetInstanceProcAddr(MwWidget handle);
MWDECL VkInstance		 MwVulkanGetInstance(MwWidget handle);
MWDECL VkSurfaceKHR		 MwVulkanGetSurface(MwWidget handle);
MWDECL VkPhysicalDevice		 MwVulkanGetPhysicalDevice(MwWidget handle);
MWDECL VkDevice			 MwVulkanGetLogicalDevice(MwWidget handle);
MWDECL VkQueue			 MwVulkanGetQueue(MwWidget handle);
MWDECL int			 MwVulkanGetGraphicsQueueIndex(MwWidget handle);

#ifdef __cplusplus
}
#endif

#endif
