//========================================================================
// Name
//     Vulkan (Cross-Platform) Extension Loader
//
// Repository
//     https://github.com/VallentinSource/vkel
//
// Overview
//     This is a simple, dynamic and tiny cross-platform Vulkan
//     extension loader.
//
// Dependencies
//     Vulkan (header and library)
//     stdlib - needed for calloc() and free()
//     Windows (header) - needed for library loading on Windows
//     dlfcn (header) - needed for library loading on non-Windows OS'
//
// Notice
//     Copyright (c) 2016 Vallentin Source <mail@vallentinsource.com>
//
// Developers & Contributors
//     Christian Vallentin <mail@vallentinsource.com>
//
// Version
//     Last Modified Data: February 24, 2016
//     Revision: 2
//
// Revision History
//     Revision 2, 2016/02/24
//       - Created a Python script for automatically generating
//         all the extensions and their functions. (Tested with
//         Python 3.5.1)
//       - Added cross-platform support, for loading libraries
//         and getting the function addresses.
//       - Fixed so platform specific functions defaults to NULL
//       - Added missing include for dlfcn (used on non-Window OS')
//
//     Revision 1, 2016/02/23
//       - Implemented the basic version supporting a few (manually
//         written) dynamically loaded functions.
//
//------------------------------------------------------------------------
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//========================================================================

#include "vkel.h"

#ifdef __cplusplus
extern "C" {
#endif

// Required for calloc() and free()
#include <stdlib.h>

#if defined(VK_USE_PLATFORM_WIN32_KHR) || defined(_WIN32) || defined(_WIN64)
#	define VKEL_USE_PLATFORM_WIN32
#endif

#ifdef VKEL_USE_PLATFORM_WIN32

#ifndef VC_EXTRALEAN
#	define VC_EXTRALEAN
#endif

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

// STOP THAT WINDOWS!
#ifndef NOMINMAX
#	define NOMINMAX
#endif

#include <Windows.h>

#else

#include <dlfcn.h>

#endif

#if defined(VKEL_USE_PLATFORM_WIN32)
#	define vkelPlatformOpenLibrary(name) LoadLibraryA(name)
#	define vkelPlatformCloseLibrary(handle) FreeLibrary((HMODULE) handle)
#	define vkelPlatformGetProcAddr(handle, name) GetProcAddress((HMODULE) handle, name)
// #elif defined(__APPLE__) || defined(__linux__) || defined(__ANDROID__) || defined(__unix__ )
#else
#define vkelPlatformOpenLibrary(name) dlopen(name, RTLD_LAZY | RTLD_LOCAL)
#define vkelPlatformCloseLibrary(handle) dlclose(handle)
#define vkelPlatformGetProcAddr(handle, name) dlsym(handle, name)
// #else
// #	error VKEL Unsupported Platform
#endif

static void *vkel_vk_lib_handle;

PFN_vkVoidFunction vkelGetProcAddr(const char *name)
{
	return (PFN_vkVoidFunction) vkelPlatformGetProcAddr(vkel_vk_lib_handle, name);
}

PFN_vkVoidFunction vkelGetInstanceProcAddr(VkInstance instance, const char *pName)
{
	PFN_vkVoidFunction proc = (PFN_vkVoidFunction) vkGetInstanceProcAddr(instance, pName);
	
	if (!proc)
		proc = (PFN_vkVoidFunction) vkelGetProcAddr(pName);
	
	return proc;
}

static int vkel_strcmp(const char *str1, const char *str2)
{
	while (*str1 && (*str1 == *str2))
	{
		str1++, str2++;
	}
	
	return *(const unsigned char*) str1 - *(const unsigned char*) str2;
}

VkBool32 vkelIsSupported(const char *extension)
{
	if (!extension)
		return VK_FALSE;
	
	uint32_t count;
	VkResult err = vkEnumerateInstanceExtensionProperties(NULL, &count, NULL);
	
	if (!err)
	{
		VkExtensionProperties *ext_props = (VkExtensionProperties*) calloc(count, sizeof(VkExtensionProperties));
		err = vkEnumerateInstanceExtensionProperties(NULL, &count, ext_props);
		
		if (!err)
		{
			for (uint32_t i = 0; i < count; i++)
			{
				if (!vkel_strcmp(extension, ext_props[i].extensionName))
				{
					free(ext_props);
					
					return VK_TRUE;
				}
			}
		}
		
		free(ext_props);
	}
	
	return VK_FALSE;
}

VkBool32 vkelInit(void)
{
	vkelUninit();
	
#ifdef VKEL_USE_PLATFORM_WIN32
	const char *name = "vulkan-1.dll";
#else
	const char *name = "libvulkan.so.1";
#endif
	
	vkel_vk_lib_handle = vkelPlatformOpenLibrary(name);
	
	if (!vkel_vk_lib_handle)
		return VK_FALSE;

	__vkAllocationFunction = (PFN_vkAllocationFunction) vkelGetProcAddr("vkAllocationFunction");
	__vkReallocationFunction = (PFN_vkReallocationFunction) vkelGetProcAddr("vkReallocationFunction");
	__vkFreeFunction = (PFN_vkFreeFunction) vkelGetProcAddr("vkFreeFunction");
	__vkInternalAllocationNotification = (PFN_vkInternalAllocationNotification) vkelGetProcAddr("vkInternalAllocationNotification");
	__vkInternalFreeNotification = (PFN_vkInternalFreeNotification) vkelGetProcAddr("vkInternalFreeNotification");
	__vkVoidFunction = (PFN_vkVoidFunction) vkelGetProcAddr("vkVoidFunction");
	__vkCreateInstance = (PFN_vkCreateInstance) vkelGetProcAddr("vkCreateInstance");
	__vkDestroyInstance = (PFN_vkDestroyInstance) vkelGetProcAddr("vkDestroyInstance");
	__vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) vkelGetProcAddr("vkEnumeratePhysicalDevices");
	__vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures) vkelGetProcAddr("vkGetPhysicalDeviceFeatures");
	__vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties) vkelGetProcAddr("vkGetPhysicalDeviceFormatProperties");
	__vkGetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties) vkelGetProcAddr("vkGetPhysicalDeviceImageFormatProperties");
	__vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) vkelGetProcAddr("vkGetPhysicalDeviceProperties");
	__vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) vkelGetProcAddr("vkGetPhysicalDeviceQueueFamilyProperties");
	__vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties) vkelGetProcAddr("vkGetPhysicalDeviceMemoryProperties");
	__vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) vkelGetProcAddr("vkGetInstanceProcAddr");
	__vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr) vkelGetProcAddr("vkGetDeviceProcAddr");
	__vkCreateDevice = (PFN_vkCreateDevice) vkelGetProcAddr("vkCreateDevice");
	__vkDestroyDevice = (PFN_vkDestroyDevice) vkelGetProcAddr("vkDestroyDevice");
	__vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties) vkelGetProcAddr("vkEnumerateInstanceExtensionProperties");
	__vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties) vkelGetProcAddr("vkEnumerateDeviceExtensionProperties");
	__vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties) vkelGetProcAddr("vkEnumerateInstanceLayerProperties");
	__vkEnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties) vkelGetProcAddr("vkEnumerateDeviceLayerProperties");
	__vkGetDeviceQueue = (PFN_vkGetDeviceQueue) vkelGetProcAddr("vkGetDeviceQueue");
	__vkQueueSubmit = (PFN_vkQueueSubmit) vkelGetProcAddr("vkQueueSubmit");
	__vkQueueWaitIdle = (PFN_vkQueueWaitIdle) vkelGetProcAddr("vkQueueWaitIdle");
	__vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle) vkelGetProcAddr("vkDeviceWaitIdle");
	__vkAllocateMemory = (PFN_vkAllocateMemory) vkelGetProcAddr("vkAllocateMemory");
	__vkFreeMemory = (PFN_vkFreeMemory) vkelGetProcAddr("vkFreeMemory");
	__vkMapMemory = (PFN_vkMapMemory) vkelGetProcAddr("vkMapMemory");
	__vkUnmapMemory = (PFN_vkUnmapMemory) vkelGetProcAddr("vkUnmapMemory");
	__vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges) vkelGetProcAddr("vkFlushMappedMemoryRanges");
	__vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges) vkelGetProcAddr("vkInvalidateMappedMemoryRanges");
	__vkGetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment) vkelGetProcAddr("vkGetDeviceMemoryCommitment");
	__vkBindBufferMemory = (PFN_vkBindBufferMemory) vkelGetProcAddr("vkBindBufferMemory");
	__vkBindImageMemory = (PFN_vkBindImageMemory) vkelGetProcAddr("vkBindImageMemory");
	__vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements) vkelGetProcAddr("vkGetBufferMemoryRequirements");
	__vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements) vkelGetProcAddr("vkGetImageMemoryRequirements");
	__vkGetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements) vkelGetProcAddr("vkGetImageSparseMemoryRequirements");
	__vkGetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties) vkelGetProcAddr("vkGetPhysicalDeviceSparseImageFormatProperties");
	__vkQueueBindSparse = (PFN_vkQueueBindSparse) vkelGetProcAddr("vkQueueBindSparse");
	__vkCreateFence = (PFN_vkCreateFence) vkelGetProcAddr("vkCreateFence");
	__vkDestroyFence = (PFN_vkDestroyFence) vkelGetProcAddr("vkDestroyFence");
	__vkResetFences = (PFN_vkResetFences) vkelGetProcAddr("vkResetFences");
	__vkGetFenceStatus = (PFN_vkGetFenceStatus) vkelGetProcAddr("vkGetFenceStatus");
	__vkWaitForFences = (PFN_vkWaitForFences) vkelGetProcAddr("vkWaitForFences");
	__vkCreateSemaphore = (PFN_vkCreateSemaphore) vkelGetProcAddr("vkCreateSemaphore");
	__vkDestroySemaphore = (PFN_vkDestroySemaphore) vkelGetProcAddr("vkDestroySemaphore");
	__vkCreateEvent = (PFN_vkCreateEvent) vkelGetProcAddr("vkCreateEvent");
	__vkDestroyEvent = (PFN_vkDestroyEvent) vkelGetProcAddr("vkDestroyEvent");
	__vkGetEventStatus = (PFN_vkGetEventStatus) vkelGetProcAddr("vkGetEventStatus");
	__vkSetEvent = (PFN_vkSetEvent) vkelGetProcAddr("vkSetEvent");
	__vkResetEvent = (PFN_vkResetEvent) vkelGetProcAddr("vkResetEvent");
	__vkCreateQueryPool = (PFN_vkCreateQueryPool) vkelGetProcAddr("vkCreateQueryPool");
	__vkDestroyQueryPool = (PFN_vkDestroyQueryPool) vkelGetProcAddr("vkDestroyQueryPool");
	__vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults) vkelGetProcAddr("vkGetQueryPoolResults");
	__vkCreateBuffer = (PFN_vkCreateBuffer) vkelGetProcAddr("vkCreateBuffer");
	__vkDestroyBuffer = (PFN_vkDestroyBuffer) vkelGetProcAddr("vkDestroyBuffer");
	__vkCreateBufferView = (PFN_vkCreateBufferView) vkelGetProcAddr("vkCreateBufferView");
	__vkDestroyBufferView = (PFN_vkDestroyBufferView) vkelGetProcAddr("vkDestroyBufferView");
	__vkCreateImage = (PFN_vkCreateImage) vkelGetProcAddr("vkCreateImage");
	__vkDestroyImage = (PFN_vkDestroyImage) vkelGetProcAddr("vkDestroyImage");
	__vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout) vkelGetProcAddr("vkGetImageSubresourceLayout");
	__vkCreateImageView = (PFN_vkCreateImageView) vkelGetProcAddr("vkCreateImageView");
	__vkDestroyImageView = (PFN_vkDestroyImageView) vkelGetProcAddr("vkDestroyImageView");
	__vkCreateShaderModule = (PFN_vkCreateShaderModule) vkelGetProcAddr("vkCreateShaderModule");
	__vkDestroyShaderModule = (PFN_vkDestroyShaderModule) vkelGetProcAddr("vkDestroyShaderModule");
	__vkCreatePipelineCache = (PFN_vkCreatePipelineCache) vkelGetProcAddr("vkCreatePipelineCache");
	__vkDestroyPipelineCache = (PFN_vkDestroyPipelineCache) vkelGetProcAddr("vkDestroyPipelineCache");
	__vkGetPipelineCacheData = (PFN_vkGetPipelineCacheData) vkelGetProcAddr("vkGetPipelineCacheData");
	__vkMergePipelineCaches = (PFN_vkMergePipelineCaches) vkelGetProcAddr("vkMergePipelineCaches");
	__vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines) vkelGetProcAddr("vkCreateGraphicsPipelines");
	__vkCreateComputePipelines = (PFN_vkCreateComputePipelines) vkelGetProcAddr("vkCreateComputePipelines");
	__vkDestroyPipeline = (PFN_vkDestroyPipeline) vkelGetProcAddr("vkDestroyPipeline");
	__vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout) vkelGetProcAddr("vkCreatePipelineLayout");
	__vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout) vkelGetProcAddr("vkDestroyPipelineLayout");
	__vkCreateSampler = (PFN_vkCreateSampler) vkelGetProcAddr("vkCreateSampler");
	__vkDestroySampler = (PFN_vkDestroySampler) vkelGetProcAddr("vkDestroySampler");
	__vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout) vkelGetProcAddr("vkCreateDescriptorSetLayout");
	__vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout) vkelGetProcAddr("vkDestroyDescriptorSetLayout");
	__vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool) vkelGetProcAddr("vkCreateDescriptorPool");
	__vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool) vkelGetProcAddr("vkDestroyDescriptorPool");
	__vkResetDescriptorPool = (PFN_vkResetDescriptorPool) vkelGetProcAddr("vkResetDescriptorPool");
	__vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets) vkelGetProcAddr("vkAllocateDescriptorSets");
	__vkFreeDescriptorSets = (PFN_vkFreeDescriptorSets) vkelGetProcAddr("vkFreeDescriptorSets");
	__vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets) vkelGetProcAddr("vkUpdateDescriptorSets");
	__vkCreateFramebuffer = (PFN_vkCreateFramebuffer) vkelGetProcAddr("vkCreateFramebuffer");
	__vkDestroyFramebuffer = (PFN_vkDestroyFramebuffer) vkelGetProcAddr("vkDestroyFramebuffer");
	__vkCreateRenderPass = (PFN_vkCreateRenderPass) vkelGetProcAddr("vkCreateRenderPass");
	__vkDestroyRenderPass = (PFN_vkDestroyRenderPass) vkelGetProcAddr("vkDestroyRenderPass");
	__vkGetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity) vkelGetProcAddr("vkGetRenderAreaGranularity");
	__vkCreateCommandPool = (PFN_vkCreateCommandPool) vkelGetProcAddr("vkCreateCommandPool");
	__vkDestroyCommandPool = (PFN_vkDestroyCommandPool) vkelGetProcAddr("vkDestroyCommandPool");
	__vkResetCommandPool = (PFN_vkResetCommandPool) vkelGetProcAddr("vkResetCommandPool");
	__vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers) vkelGetProcAddr("vkAllocateCommandBuffers");
	__vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers) vkelGetProcAddr("vkFreeCommandBuffers");
	__vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer) vkelGetProcAddr("vkBeginCommandBuffer");
	__vkEndCommandBuffer = (PFN_vkEndCommandBuffer) vkelGetProcAddr("vkEndCommandBuffer");
	__vkResetCommandBuffer = (PFN_vkResetCommandBuffer) vkelGetProcAddr("vkResetCommandBuffer");
	__vkCmdBindPipeline = (PFN_vkCmdBindPipeline) vkelGetProcAddr("vkCmdBindPipeline");
	__vkCmdSetViewport = (PFN_vkCmdSetViewport) vkelGetProcAddr("vkCmdSetViewport");
	__vkCmdSetScissor = (PFN_vkCmdSetScissor) vkelGetProcAddr("vkCmdSetScissor");
	__vkCmdSetLineWidth = (PFN_vkCmdSetLineWidth) vkelGetProcAddr("vkCmdSetLineWidth");
	__vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias) vkelGetProcAddr("vkCmdSetDepthBias");
	__vkCmdSetBlendConstants = (PFN_vkCmdSetBlendConstants) vkelGetProcAddr("vkCmdSetBlendConstants");
	__vkCmdSetDepthBounds = (PFN_vkCmdSetDepthBounds) vkelGetProcAddr("vkCmdSetDepthBounds");
	__vkCmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask) vkelGetProcAddr("vkCmdSetStencilCompareMask");
	__vkCmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask) vkelGetProcAddr("vkCmdSetStencilWriteMask");
	__vkCmdSetStencilReference = (PFN_vkCmdSetStencilReference) vkelGetProcAddr("vkCmdSetStencilReference");
	__vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets) vkelGetProcAddr("vkCmdBindDescriptorSets");
	__vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer) vkelGetProcAddr("vkCmdBindIndexBuffer");
	__vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers) vkelGetProcAddr("vkCmdBindVertexBuffers");
	__vkCmdDraw = (PFN_vkCmdDraw) vkelGetProcAddr("vkCmdDraw");
	__vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed) vkelGetProcAddr("vkCmdDrawIndexed");
	__vkCmdDrawIndirect = (PFN_vkCmdDrawIndirect) vkelGetProcAddr("vkCmdDrawIndirect");
	__vkCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect) vkelGetProcAddr("vkCmdDrawIndexedIndirect");
	__vkCmdDispatch = (PFN_vkCmdDispatch) vkelGetProcAddr("vkCmdDispatch");
	__vkCmdDispatchIndirect = (PFN_vkCmdDispatchIndirect) vkelGetProcAddr("vkCmdDispatchIndirect");
	__vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer) vkelGetProcAddr("vkCmdCopyBuffer");
	__vkCmdCopyImage = (PFN_vkCmdCopyImage) vkelGetProcAddr("vkCmdCopyImage");
	__vkCmdBlitImage = (PFN_vkCmdBlitImage) vkelGetProcAddr("vkCmdBlitImage");
	__vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage) vkelGetProcAddr("vkCmdCopyBufferToImage");
	__vkCmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer) vkelGetProcAddr("vkCmdCopyImageToBuffer");
	__vkCmdUpdateBuffer = (PFN_vkCmdUpdateBuffer) vkelGetProcAddr("vkCmdUpdateBuffer");
	__vkCmdFillBuffer = (PFN_vkCmdFillBuffer) vkelGetProcAddr("vkCmdFillBuffer");
	__vkCmdClearColorImage = (PFN_vkCmdClearColorImage) vkelGetProcAddr("vkCmdClearColorImage");
	__vkCmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage) vkelGetProcAddr("vkCmdClearDepthStencilImage");
	__vkCmdClearAttachments = (PFN_vkCmdClearAttachments) vkelGetProcAddr("vkCmdClearAttachments");
	__vkCmdResolveImage = (PFN_vkCmdResolveImage) vkelGetProcAddr("vkCmdResolveImage");
	__vkCmdSetEvent = (PFN_vkCmdSetEvent) vkelGetProcAddr("vkCmdSetEvent");
	__vkCmdResetEvent = (PFN_vkCmdResetEvent) vkelGetProcAddr("vkCmdResetEvent");
	__vkCmdWaitEvents = (PFN_vkCmdWaitEvents) vkelGetProcAddr("vkCmdWaitEvents");
	__vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier) vkelGetProcAddr("vkCmdPipelineBarrier");
	__vkCmdBeginQuery = (PFN_vkCmdBeginQuery) vkelGetProcAddr("vkCmdBeginQuery");
	__vkCmdEndQuery = (PFN_vkCmdEndQuery) vkelGetProcAddr("vkCmdEndQuery");
	__vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool) vkelGetProcAddr("vkCmdResetQueryPool");
	__vkCmdWriteTimestamp = (PFN_vkCmdWriteTimestamp) vkelGetProcAddr("vkCmdWriteTimestamp");
	__vkCmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults) vkelGetProcAddr("vkCmdCopyQueryPoolResults");
	__vkCmdPushConstants = (PFN_vkCmdPushConstants) vkelGetProcAddr("vkCmdPushConstants");
	__vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass) vkelGetProcAddr("vkCmdBeginRenderPass");
	__vkCmdNextSubpass = (PFN_vkCmdNextSubpass) vkelGetProcAddr("vkCmdNextSubpass");
	__vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass) vkelGetProcAddr("vkCmdEndRenderPass");
	__vkCmdExecuteCommands = (PFN_vkCmdExecuteCommands) vkelGetProcAddr("vkCmdExecuteCommands");
	__vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR) vkelGetProcAddr("vkDestroySurfaceKHR");
	__vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR) vkelGetProcAddr("vkGetPhysicalDeviceSurfaceSupportKHR");
	__vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) vkelGetProcAddr("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	__vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) vkelGetProcAddr("vkGetPhysicalDeviceSurfaceFormatsKHR");
	__vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) vkelGetProcAddr("vkGetPhysicalDeviceSurfacePresentModesKHR");
	__vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR) vkelGetProcAddr("vkCreateSwapchainKHR");
	__vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR) vkelGetProcAddr("vkDestroySwapchainKHR");
	__vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) vkelGetProcAddr("vkGetSwapchainImagesKHR");
	__vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR) vkelGetProcAddr("vkAcquireNextImageKHR");
	__vkQueuePresentKHR = (PFN_vkQueuePresentKHR) vkelGetProcAddr("vkQueuePresentKHR");
	__vkGetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR) vkelGetProcAddr("vkGetPhysicalDeviceDisplayPropertiesKHR");
	__vkGetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR) vkelGetProcAddr("vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
	__vkGetDisplayPlaneSupportedDisplaysKHR = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR) vkelGetProcAddr("vkGetDisplayPlaneSupportedDisplaysKHR");
	__vkGetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR) vkelGetProcAddr("vkGetDisplayModePropertiesKHR");
	__vkCreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR) vkelGetProcAddr("vkCreateDisplayModeKHR");
	__vkGetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR) vkelGetProcAddr("vkGetDisplayPlaneCapabilitiesKHR");
	__vkCreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR) vkelGetProcAddr("vkCreateDisplayPlaneSurfaceKHR");
	__vkCreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR) vkelGetProcAddr("vkCreateSharedSwapchainsKHR");
	__vkDebugReportCallbackEXT = (PFN_vkDebugReportCallbackEXT) vkelGetProcAddr("vkDebugReportCallbackEXT");
	__vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) vkelGetProcAddr("vkCreateDebugReportCallbackEXT");
	__vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) vkelGetProcAddr("vkDestroyDebugReportCallbackEXT");
	__vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT) vkelGetProcAddr("vkDebugReportMessageEXT");

#ifdef VK_USE_PLATFORM_ANDROID_KHR
	__vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR) vkelGetProcAddr("vkCreateAndroidSurfaceKHR");
#endif /* VK_USE_PLATFORM_ANDROID_KHR */

#ifdef VK_USE_PLATFORM_MIR_KHR
	__vkCreateMirSurfaceKHR = (PFN_vkCreateMirSurfaceKHR) vkelGetProcAddr("vkCreateMirSurfaceKHR");
	__vkGetPhysicalDeviceMirPresentationSupportKHR = (PFN_vkGetPhysicalDeviceMirPresentationSupportKHR) vkelGetProcAddr("vkGetPhysicalDeviceMirPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_MIR_KHR */

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	__vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR) vkelGetProcAddr("vkCreateWaylandSurfaceKHR");
	__vkGetPhysicalDeviceWaylandPresentationSupportKHR = (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR) vkelGetProcAddr("vkGetPhysicalDeviceWaylandPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_WAYLAND_KHR */

#ifdef VK_USE_PLATFORM_WIN32_KHR
	__vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR) vkelGetProcAddr("vkCreateWin32SurfaceKHR");
	__vkGetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR) vkelGetProcAddr("vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif /* VK_USE_PLATFORM_WIN32_KHR */

#ifdef VK_USE_PLATFORM_XCB_KHR
	__vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR) vkelGetProcAddr("vkCreateXcbSurfaceKHR");
	__vkGetPhysicalDeviceXcbPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR) vkelGetProcAddr("vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_XCB_KHR */

#ifdef VK_USE_PLATFORM_XLIB_KHR
	__vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR) vkelGetProcAddr("vkCreateXlibSurfaceKHR");
	__vkGetPhysicalDeviceXlibPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR) vkelGetProcAddr("vkGetPhysicalDeviceXlibPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_XLIB_KHR */

	VKEL_KHR_surface = vkelIsSupported("VK_KHR_surface");
	VKEL_KHR_swapchain = vkelIsSupported("VK_KHR_swapchain");
	VKEL_KHR_display = vkelIsSupported("VK_KHR_display");
	VKEL_KHR_display_swapchain = vkelIsSupported("VK_KHR_display_swapchain");
	VKEL_KHR_xlib_surface = vkelIsSupported("VK_KHR_xlib_surface");
	VKEL_KHR_xcb_surface = vkelIsSupported("VK_KHR_xcb_surface");
	VKEL_KHR_wayland_surface = vkelIsSupported("VK_KHR_wayland_surface");
	VKEL_KHR_mir_surface = vkelIsSupported("VK_KHR_mir_surface");
	VKEL_KHR_android_surface = vkelIsSupported("VK_KHR_android_surface");
	VKEL_KHR_win32_surface = vkelIsSupported("VK_KHR_win32_surface");
	VKEL_EXT_debug_report = vkelIsSupported("VK_EXT_debug_report");

	return VK_TRUE;
}

void vkelUninit(void)
{
	if (vkel_vk_lib_handle)
	{
		vkelPlatformCloseLibrary(vkel_vk_lib_handle);
		
		vkel_vk_lib_handle = NULL;
	}
}

#ifdef __cplusplus
}
#endif