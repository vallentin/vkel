//========================================================================
// Name
//     Vulkan (Cross-Platform) Extension Loader
//
// Repository
//     https://github.com/MrVallentin/vkel
//
// Overview
//     This is a simple, dynamic and tiny cross-platform Vulkan
//     extension loader.
//
// Dependencies
//     Vulkan (library)
//     Windows (header) - needed for library loading on Windows
//     dlfcn (header) - needed for library loading on non-Windows OS'
//     Standard C Libraries (stdio, stdlib, string, assert) - needed for NULL, malloc()
//                                                 calloc(), free(), memset(), assert()
//
// Notice
//     Copyright (c) 2016 Christian Vallentin <mail@vallentinsource.com>
//
// Developers & Contributors
//     Christian Vallentin <mail@vallentinsource.com>
//
// Version History
//     Last Modified Date: April 03, 2016
//     Revision: 7
//     Version: 2.0.4
//
// Revision History
//     Revision 7, 2016/04/03
//       - Updated support for Vulkan 1.0.7
//
//     Revision 6, 2016/03/26
//       - Updated support for Vulkan 1.0.7
//
//     Revision 5, 2016/03/12
//       - Updated support for Vulkan 1.0.6
//
//     Revision 4, 2016/03/06
//       - Updated support for Vulkan 1.0.5
//
//     Revision 3, 2016/02/26
//       - Rewrote vkel_gen.py, now it parses and directly
//         adds vulkan.h and vk_platform.h into vkel.h,
//         along with moving the appropriate copyrights
//         to the top of vkel.h.
//       - Fixed/added better differentiation for instance
//         and device related calls.
//       - Removed the need for having the vukan.h and
//         vk_platform.h headers.
//       - Updated support for Vulkan 1.0.4
//
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

/*
** Copyright (c) 2015-2016 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

#include "vkel.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h> /* NULL, printf() */
#include <stdlib.h> /* malloc(), calloc(), free() */
#include <string.h> /* memset() */
#include <assert.h> /* assert() */


#ifdef VK_USE_PLATFORM_WIN32_KHR

#ifndef VC_EXTRALEAN
#	define VC_EXTRALEAN
#endif

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

// STOP THAT, BAD WINDOWS!
#ifndef NOMINMAX
#	define NOMINMAX
#endif

#include <Windows.h>
#else

#include <dlfcn.h>

#endif


#ifdef VK_USE_PLATFORM_WIN32_KHR
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

static void *vkelVkLibHandle;



PFN_vkVoidFunction vkelGetProcAddr(const char *name)
{
	return (PFN_vkVoidFunction) vkelPlatformGetProcAddr(vkelVkLibHandle, name);
}

PFN_vkVoidFunction vkelGetInstanceProcAddr(VkInstance instance, const char *pName)
{
	PFN_vkVoidFunction proc = (PFN_vkVoidFunction) vkGetInstanceProcAddr(instance, pName);
	
	if (!proc)
		proc = (PFN_vkVoidFunction) vkelGetProcAddr(pName);
	
	return proc;
}

PFN_vkVoidFunction vkelGetDeviceProcAddr(VkDevice device, const char *pName)
{
	PFN_vkVoidFunction proc = (PFN_vkVoidFunction) vkGetDeviceProcAddr(device, pName);
	
	if (!proc)
		proc = (PFN_vkVoidFunction) vkelGetInstanceProcAddr(NULL, pName);
	
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

static void vkel_strpy(char *dest, char *src)
{
	while (*dest++ = *src++);
}


void vkelDeleteNames(uint32_t nameCount, char **names)
{
	// assert(names);
	if (!names)
		return;

	for (uint32_t nameIndex = 0; nameIndex < nameCount; nameIndex++)
	{
		// assert(names[nameIndex]);
		if (!names[nameIndex])
			continue;

		free(names[nameIndex]);
	}

	free(names);
}


char** vkelGetInstanceExtensionNames(const char *pLayerName, uint32_t *extensionNameCount)
{
	assert(extensionNameCount);


	VkResult err;


	uint32_t extPropertyCount;
	err = vkEnumerateInstanceExtensionProperties(pLayerName, &extPropertyCount, NULL);
	assert(!err);

	(*extensionNameCount) = extPropertyCount;

	if (extPropertyCount < 1)
		return NULL;


	char **extensionNames = (char**) calloc(extPropertyCount, sizeof(char*));

	for (uint32_t extensionNameIndex = 0; extensionNameIndex < extPropertyCount; extensionNameIndex++)
	{
		extensionNames[extensionNameIndex] = (char*)calloc(VK_MAX_EXTENSION_NAME_SIZE, sizeof(char));
	}


	VkExtensionProperties *extProperties = (VkExtensionProperties*) calloc(extPropertyCount, sizeof(VkExtensionProperties));
	assert(extProperties);


	err = vkEnumerateInstanceExtensionProperties(pLayerName, &extPropertyCount, extProperties);
	assert(!err);

	for (uint32_t extPropertyIndex = 0; extPropertyIndex < extPropertyCount; extPropertyIndex++)
	{
		VkExtensionProperties extProperty = extProperties[extPropertyIndex];

		vkel_strpy(extensionNames[extPropertyIndex], extProperty.extensionName);
	}


	free(extProperties);


	return extensionNames;
}

char** vkelGetInstanceLayerNames(uint32_t *layerNameCount)
{
	assert(layerNameCount);


	VkResult err;


	uint32_t layerPropertyCount;
	err = vkEnumerateInstanceLayerProperties(&layerPropertyCount, NULL);
	assert(!err);

	(*layerNameCount) = layerPropertyCount;

	if (layerPropertyCount < 1)
		return NULL;


	char **layerNames = (char**) calloc(layerPropertyCount, sizeof(char*));

	for (uint32_t layerNameIndex = 0; layerNameIndex < layerPropertyCount; layerNameIndex++)
	{
		layerNames[layerNameIndex] = (char*) calloc(VK_MAX_EXTENSION_NAME_SIZE, sizeof(char));
	}


	VkLayerProperties *layerProperties = (VkLayerProperties*) calloc(layerPropertyCount, sizeof(VkLayerProperties));
	assert(layerProperties);

	err = vkEnumerateInstanceLayerProperties(&layerPropertyCount, layerProperties);
	assert(!err);

	for (uint32_t layerPropertyIndex = 0; layerPropertyIndex < layerPropertyCount; layerPropertyIndex++)
	{
		VkLayerProperties layerProperty = layerProperties[layerPropertyIndex];

		vkel_strpy(layerNames[layerPropertyIndex], layerProperty.layerName);
	}


	free(layerProperties);


	return layerNames;
}


char** vkelGetDeviceExtensionNames(VkPhysicalDevice physicalDevice, const char *pLayerName, uint32_t *extensionNameCount)
{
	assert(extensionNameCount);


	VkResult err;


	uint32_t extPropertyCount;
	err = vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, &extPropertyCount, NULL);
	assert(!err);

	(*extensionNameCount) = extPropertyCount;

	if (extPropertyCount < 1)
		return NULL;


	char **extensionNames = (char**) calloc(extPropertyCount, sizeof(char*));

	for (uint32_t extensionNameIndex = 0; extensionNameIndex < extPropertyCount; extensionNameIndex++)
	{
		extensionNames[extensionNameIndex] = (char*) calloc(VK_MAX_EXTENSION_NAME_SIZE, sizeof(char));
	}


	VkExtensionProperties *extProperties = (VkExtensionProperties*) calloc(extPropertyCount, sizeof(VkExtensionProperties));
	assert(extProperties);


	err = vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, &extPropertyCount, extProperties);
	assert(!err);

	for (uint32_t extPropertyIndex = 0; extPropertyIndex < extPropertyCount; extPropertyIndex++)
	{
		VkExtensionProperties extProperty = extProperties[extPropertyIndex];

		vkel_strpy(extensionNames[extPropertyIndex], extProperty.extensionName);
	}


	free(extProperties);


	return extensionNames;
}

char** vkelGetDeviceLayerNames(VkPhysicalDevice physicalDevice, uint32_t *layerNameCount)
{
	assert(layerNameCount);


	VkResult err;


	uint32_t layerPropertyCount;
	err = vkEnumerateDeviceLayerProperties(physicalDevice, &layerPropertyCount, NULL);
	assert(!err);

	(*layerNameCount) = layerPropertyCount;

	if (layerPropertyCount < 1)
		return NULL;


	char **layerNames = (char**) calloc(layerPropertyCount, sizeof(char*));

	for (uint32_t layerNameIndex = 0; layerNameIndex < layerPropertyCount; layerNameIndex++)
	{
		layerNames[layerNameIndex] = (char*) calloc(VK_MAX_EXTENSION_NAME_SIZE, sizeof(char));
	}


	VkLayerProperties *layerProperties = (VkLayerProperties*) calloc(layerPropertyCount, sizeof(VkLayerProperties));
	assert(layerProperties);

	err = vkEnumerateDeviceLayerProperties(physicalDevice, &layerPropertyCount, layerProperties);
	assert(!err);

	for (uint32_t layerPropertyIndex = 0; layerPropertyIndex < layerPropertyCount; layerPropertyIndex++)
	{
		VkLayerProperties layerProperty = layerProperties[layerPropertyIndex];

		vkel_strpy(layerNames[layerPropertyIndex], layerProperty.layerName);
	}


	free(layerProperties);


	return layerNames;
}


VkBool32 vkelIsInstanceLayerSupported(const char *pLayerName)
{
	uint32_t layerNameCount = 0;
	char **layerNames = vkelGetInstanceLayerNames(&layerNameCount);

	for (uint32_t layerNameIndex = 0; layerNameIndex < layerNameCount; layerNameIndex++)
	{
		if (vkel_strcmp(layerNames[layerNameIndex], pLayerName))
		{
			vkelDeleteInstanceLayerNames(layerNameCount, layerNames);

			return VK_TRUE;
		}
	}

	vkelDeleteInstanceLayerNames(layerNameCount, layerNames);

	return VK_FALSE;
}

VkBool32 vkelIsInstanceExtensionSupported(const char *pLayerName, const char *pExtensionName)
{
	uint32_t extensionNameCount = 0;
	char **extensionNames = vkelGetInstanceExtensionNames(pLayerName, &extensionNameCount);

	for (uint32_t extensionNameIndex = 0; extensionNameIndex < extensionNameCount; extensionNameIndex++)
	{
		if (vkel_strcmp(extensionNames[extensionNameIndex], pExtensionName))
		{
			vkelDeleteInstanceExtensionNames(extensionNameCount, extensionNames);

			return VK_TRUE;
		}
	}

	vkelDeleteInstanceExtensionNames(extensionNameCount, extensionNames);

	return VK_FALSE;
}


VkBool32 vkelIsDeviceLayerSupported(VkPhysicalDevice physicalDevice, const char *pLayerName)
{
	uint32_t layerNameCount = 0;
	char **layerNames = vkelGetDeviceLayerNames(physicalDevice, &layerNameCount);

	for (uint32_t layerNameIndex = 0; layerNameIndex < layerNameCount; layerNameIndex++)
	{
		if (vkel_strcmp(layerNames[layerNameIndex], pLayerName))
		{
			vkelDeleteDeviceLayerNames(layerNameCount, layerNames);

			return VK_TRUE;
		}
	}

	vkelDeleteDeviceLayerNames(layerNameCount, layerNames);

	return VK_FALSE;
}

VkBool32 vkelIsDeviceExtensionSupported(VkPhysicalDevice physicalDevice, const char *pLayerName, const char *pExtensionName)
{
	uint32_t extensionNameCount = 0;
	char **extensionNames = vkelGetDeviceExtensionNames(physicalDevice, pLayerName, &extensionNameCount);

	for (uint32_t extensionNameIndex = 0; extensionNameIndex < extensionNameCount; extensionNameIndex++)
	{
		if (vkel_strcmp(extensionNames[extensionNameIndex], pExtensionName))
		{
			vkelDeleteDeviceExtensionNames(extensionNameCount, extensionNames);

			return VK_TRUE;
		}
	}

	vkelDeleteDeviceExtensionNames(extensionNameCount, extensionNames);

	return VK_FALSE;
}


VkBool32 vkelInit(void)
{
	vkelUninit();
	
#ifdef VK_USE_PLATFORM_WIN32_KHR
	const char *name = "vulkan-1.dll";
#else
	const char *name = "libvulkan.so.1";
#endif
	
	vkelVkLibHandle = vkelPlatformOpenLibrary(name);
	
	if (!vkelVkLibHandle)
		return VK_FALSE;
	
	
	__vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR) vkelGetProcAddr("vkAcquireNextImageKHR");
	__vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers) vkelGetProcAddr("vkAllocateCommandBuffers");
	__vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets) vkelGetProcAddr("vkAllocateDescriptorSets");
	__vkAllocateMemory = (PFN_vkAllocateMemory) vkelGetProcAddr("vkAllocateMemory");
	__vkAllocationFunction = (PFN_vkAllocationFunction) vkelGetProcAddr("vkAllocationFunction");
	__vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer) vkelGetProcAddr("vkBeginCommandBuffer");
	__vkBindBufferMemory = (PFN_vkBindBufferMemory) vkelGetProcAddr("vkBindBufferMemory");
	__vkBindImageMemory = (PFN_vkBindImageMemory) vkelGetProcAddr("vkBindImageMemory");
	__vkCmdBeginQuery = (PFN_vkCmdBeginQuery) vkelGetProcAddr("vkCmdBeginQuery");
	__vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass) vkelGetProcAddr("vkCmdBeginRenderPass");
	__vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets) vkelGetProcAddr("vkCmdBindDescriptorSets");
	__vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer) vkelGetProcAddr("vkCmdBindIndexBuffer");
	__vkCmdBindPipeline = (PFN_vkCmdBindPipeline) vkelGetProcAddr("vkCmdBindPipeline");
	__vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers) vkelGetProcAddr("vkCmdBindVertexBuffers");
	__vkCmdBlitImage = (PFN_vkCmdBlitImage) vkelGetProcAddr("vkCmdBlitImage");
	__vkCmdClearAttachments = (PFN_vkCmdClearAttachments) vkelGetProcAddr("vkCmdClearAttachments");
	__vkCmdClearColorImage = (PFN_vkCmdClearColorImage) vkelGetProcAddr("vkCmdClearColorImage");
	__vkCmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage) vkelGetProcAddr("vkCmdClearDepthStencilImage");
	__vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer) vkelGetProcAddr("vkCmdCopyBuffer");
	__vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage) vkelGetProcAddr("vkCmdCopyBufferToImage");
	__vkCmdCopyImage = (PFN_vkCmdCopyImage) vkelGetProcAddr("vkCmdCopyImage");
	__vkCmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer) vkelGetProcAddr("vkCmdCopyImageToBuffer");
	__vkCmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults) vkelGetProcAddr("vkCmdCopyQueryPoolResults");
	__vkCmdDispatch = (PFN_vkCmdDispatch) vkelGetProcAddr("vkCmdDispatch");
	__vkCmdDispatchIndirect = (PFN_vkCmdDispatchIndirect) vkelGetProcAddr("vkCmdDispatchIndirect");
	__vkCmdDraw = (PFN_vkCmdDraw) vkelGetProcAddr("vkCmdDraw");
	__vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed) vkelGetProcAddr("vkCmdDrawIndexed");
	__vkCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect) vkelGetProcAddr("vkCmdDrawIndexedIndirect");
	__vkCmdDrawIndirect = (PFN_vkCmdDrawIndirect) vkelGetProcAddr("vkCmdDrawIndirect");
	__vkCmdEndQuery = (PFN_vkCmdEndQuery) vkelGetProcAddr("vkCmdEndQuery");
	__vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass) vkelGetProcAddr("vkCmdEndRenderPass");
	__vkCmdExecuteCommands = (PFN_vkCmdExecuteCommands) vkelGetProcAddr("vkCmdExecuteCommands");
	__vkCmdFillBuffer = (PFN_vkCmdFillBuffer) vkelGetProcAddr("vkCmdFillBuffer");
	__vkCmdNextSubpass = (PFN_vkCmdNextSubpass) vkelGetProcAddr("vkCmdNextSubpass");
	__vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier) vkelGetProcAddr("vkCmdPipelineBarrier");
	__vkCmdPushConstants = (PFN_vkCmdPushConstants) vkelGetProcAddr("vkCmdPushConstants");
	__vkCmdResetEvent = (PFN_vkCmdResetEvent) vkelGetProcAddr("vkCmdResetEvent");
	__vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool) vkelGetProcAddr("vkCmdResetQueryPool");
	__vkCmdResolveImage = (PFN_vkCmdResolveImage) vkelGetProcAddr("vkCmdResolveImage");
	__vkCmdSetBlendConstants = (PFN_vkCmdSetBlendConstants) vkelGetProcAddr("vkCmdSetBlendConstants");
	__vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias) vkelGetProcAddr("vkCmdSetDepthBias");
	__vkCmdSetDepthBounds = (PFN_vkCmdSetDepthBounds) vkelGetProcAddr("vkCmdSetDepthBounds");
	__vkCmdSetEvent = (PFN_vkCmdSetEvent) vkelGetProcAddr("vkCmdSetEvent");
	__vkCmdSetLineWidth = (PFN_vkCmdSetLineWidth) vkelGetProcAddr("vkCmdSetLineWidth");
	__vkCmdSetScissor = (PFN_vkCmdSetScissor) vkelGetProcAddr("vkCmdSetScissor");
	__vkCmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask) vkelGetProcAddr("vkCmdSetStencilCompareMask");
	__vkCmdSetStencilReference = (PFN_vkCmdSetStencilReference) vkelGetProcAddr("vkCmdSetStencilReference");
	__vkCmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask) vkelGetProcAddr("vkCmdSetStencilWriteMask");
	__vkCmdSetViewport = (PFN_vkCmdSetViewport) vkelGetProcAddr("vkCmdSetViewport");
	__vkCmdUpdateBuffer = (PFN_vkCmdUpdateBuffer) vkelGetProcAddr("vkCmdUpdateBuffer");
	__vkCmdWaitEvents = (PFN_vkCmdWaitEvents) vkelGetProcAddr("vkCmdWaitEvents");
	__vkCmdWriteTimestamp = (PFN_vkCmdWriteTimestamp) vkelGetProcAddr("vkCmdWriteTimestamp");
	__vkCreateBuffer = (PFN_vkCreateBuffer) vkelGetProcAddr("vkCreateBuffer");
	__vkCreateBufferView = (PFN_vkCreateBufferView) vkelGetProcAddr("vkCreateBufferView");
	__vkCreateCommandPool = (PFN_vkCreateCommandPool) vkelGetProcAddr("vkCreateCommandPool");
	__vkCreateComputePipelines = (PFN_vkCreateComputePipelines) vkelGetProcAddr("vkCreateComputePipelines");
	__vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) vkelGetProcAddr("vkCreateDebugReportCallbackEXT");
	__vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool) vkelGetProcAddr("vkCreateDescriptorPool");
	__vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout) vkelGetProcAddr("vkCreateDescriptorSetLayout");
	__vkCreateDevice = (PFN_vkCreateDevice) vkelGetProcAddr("vkCreateDevice");
	__vkCreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR) vkelGetProcAddr("vkCreateDisplayModeKHR");
	__vkCreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR) vkelGetProcAddr("vkCreateDisplayPlaneSurfaceKHR");
	__vkCreateEvent = (PFN_vkCreateEvent) vkelGetProcAddr("vkCreateEvent");
	__vkCreateFence = (PFN_vkCreateFence) vkelGetProcAddr("vkCreateFence");
	__vkCreateFramebuffer = (PFN_vkCreateFramebuffer) vkelGetProcAddr("vkCreateFramebuffer");
	__vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines) vkelGetProcAddr("vkCreateGraphicsPipelines");
	__vkCreateImage = (PFN_vkCreateImage) vkelGetProcAddr("vkCreateImage");
	__vkCreateImageView = (PFN_vkCreateImageView) vkelGetProcAddr("vkCreateImageView");
	__vkCreateInstance = (PFN_vkCreateInstance) vkelGetProcAddr("vkCreateInstance");
	__vkCreatePipelineCache = (PFN_vkCreatePipelineCache) vkelGetProcAddr("vkCreatePipelineCache");
	__vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout) vkelGetProcAddr("vkCreatePipelineLayout");
	__vkCreateQueryPool = (PFN_vkCreateQueryPool) vkelGetProcAddr("vkCreateQueryPool");
	__vkCreateRenderPass = (PFN_vkCreateRenderPass) vkelGetProcAddr("vkCreateRenderPass");
	__vkCreateSampler = (PFN_vkCreateSampler) vkelGetProcAddr("vkCreateSampler");
	__vkCreateSemaphore = (PFN_vkCreateSemaphore) vkelGetProcAddr("vkCreateSemaphore");
	__vkCreateShaderModule = (PFN_vkCreateShaderModule) vkelGetProcAddr("vkCreateShaderModule");
	__vkCreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR) vkelGetProcAddr("vkCreateSharedSwapchainsKHR");
	__vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR) vkelGetProcAddr("vkCreateSwapchainKHR");
	__vkDebugReportCallbackEXT = (PFN_vkDebugReportCallbackEXT) vkelGetProcAddr("vkDebugReportCallbackEXT");
	__vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT) vkelGetProcAddr("vkDebugReportMessageEXT");
	__vkDestroyBuffer = (PFN_vkDestroyBuffer) vkelGetProcAddr("vkDestroyBuffer");
	__vkDestroyBufferView = (PFN_vkDestroyBufferView) vkelGetProcAddr("vkDestroyBufferView");
	__vkDestroyCommandPool = (PFN_vkDestroyCommandPool) vkelGetProcAddr("vkDestroyCommandPool");
	__vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) vkelGetProcAddr("vkDestroyDebugReportCallbackEXT");
	__vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool) vkelGetProcAddr("vkDestroyDescriptorPool");
	__vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout) vkelGetProcAddr("vkDestroyDescriptorSetLayout");
	__vkDestroyDevice = (PFN_vkDestroyDevice) vkelGetProcAddr("vkDestroyDevice");
	__vkDestroyEvent = (PFN_vkDestroyEvent) vkelGetProcAddr("vkDestroyEvent");
	__vkDestroyFence = (PFN_vkDestroyFence) vkelGetProcAddr("vkDestroyFence");
	__vkDestroyFramebuffer = (PFN_vkDestroyFramebuffer) vkelGetProcAddr("vkDestroyFramebuffer");
	__vkDestroyImage = (PFN_vkDestroyImage) vkelGetProcAddr("vkDestroyImage");
	__vkDestroyImageView = (PFN_vkDestroyImageView) vkelGetProcAddr("vkDestroyImageView");
	__vkDestroyInstance = (PFN_vkDestroyInstance) vkelGetProcAddr("vkDestroyInstance");
	__vkDestroyPipeline = (PFN_vkDestroyPipeline) vkelGetProcAddr("vkDestroyPipeline");
	__vkDestroyPipelineCache = (PFN_vkDestroyPipelineCache) vkelGetProcAddr("vkDestroyPipelineCache");
	__vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout) vkelGetProcAddr("vkDestroyPipelineLayout");
	__vkDestroyQueryPool = (PFN_vkDestroyQueryPool) vkelGetProcAddr("vkDestroyQueryPool");
	__vkDestroyRenderPass = (PFN_vkDestroyRenderPass) vkelGetProcAddr("vkDestroyRenderPass");
	__vkDestroySampler = (PFN_vkDestroySampler) vkelGetProcAddr("vkDestroySampler");
	__vkDestroySemaphore = (PFN_vkDestroySemaphore) vkelGetProcAddr("vkDestroySemaphore");
	__vkDestroyShaderModule = (PFN_vkDestroyShaderModule) vkelGetProcAddr("vkDestroyShaderModule");
	__vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR) vkelGetProcAddr("vkDestroySurfaceKHR");
	__vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR) vkelGetProcAddr("vkDestroySwapchainKHR");
	__vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle) vkelGetProcAddr("vkDeviceWaitIdle");
	__vkEndCommandBuffer = (PFN_vkEndCommandBuffer) vkelGetProcAddr("vkEndCommandBuffer");
	__vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties) vkelGetProcAddr("vkEnumerateDeviceExtensionProperties");
	__vkEnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties) vkelGetProcAddr("vkEnumerateDeviceLayerProperties");
	__vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties) vkelGetProcAddr("vkEnumerateInstanceExtensionProperties");
	__vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties) vkelGetProcAddr("vkEnumerateInstanceLayerProperties");
	__vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) vkelGetProcAddr("vkEnumeratePhysicalDevices");
	__vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges) vkelGetProcAddr("vkFlushMappedMemoryRanges");
	__vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers) vkelGetProcAddr("vkFreeCommandBuffers");
	__vkFreeDescriptorSets = (PFN_vkFreeDescriptorSets) vkelGetProcAddr("vkFreeDescriptorSets");
	__vkFreeFunction = (PFN_vkFreeFunction) vkelGetProcAddr("vkFreeFunction");
	__vkFreeMemory = (PFN_vkFreeMemory) vkelGetProcAddr("vkFreeMemory");
	__vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements) vkelGetProcAddr("vkGetBufferMemoryRequirements");
	__vkGetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment) vkelGetProcAddr("vkGetDeviceMemoryCommitment");
	__vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr) vkelGetProcAddr("vkGetDeviceProcAddr");
	__vkGetDeviceQueue = (PFN_vkGetDeviceQueue) vkelGetProcAddr("vkGetDeviceQueue");
	__vkGetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR) vkelGetProcAddr("vkGetDisplayModePropertiesKHR");
	__vkGetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR) vkelGetProcAddr("vkGetDisplayPlaneCapabilitiesKHR");
	__vkGetDisplayPlaneSupportedDisplaysKHR = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR) vkelGetProcAddr("vkGetDisplayPlaneSupportedDisplaysKHR");
	__vkGetEventStatus = (PFN_vkGetEventStatus) vkelGetProcAddr("vkGetEventStatus");
	__vkGetFenceStatus = (PFN_vkGetFenceStatus) vkelGetProcAddr("vkGetFenceStatus");
	__vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements) vkelGetProcAddr("vkGetImageMemoryRequirements");
	__vkGetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements) vkelGetProcAddr("vkGetImageSparseMemoryRequirements");
	__vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout) vkelGetProcAddr("vkGetImageSubresourceLayout");
	__vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) vkelGetProcAddr("vkGetInstanceProcAddr");
	__vkGetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR) vkelGetProcAddr("vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
	__vkGetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR) vkelGetProcAddr("vkGetPhysicalDeviceDisplayPropertiesKHR");
	__vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures) vkelGetProcAddr("vkGetPhysicalDeviceFeatures");
	__vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties) vkelGetProcAddr("vkGetPhysicalDeviceFormatProperties");
	__vkGetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties) vkelGetProcAddr("vkGetPhysicalDeviceImageFormatProperties");
	__vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties) vkelGetProcAddr("vkGetPhysicalDeviceMemoryProperties");
	__vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) vkelGetProcAddr("vkGetPhysicalDeviceProperties");
	__vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) vkelGetProcAddr("vkGetPhysicalDeviceQueueFamilyProperties");
	__vkGetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties) vkelGetProcAddr("vkGetPhysicalDeviceSparseImageFormatProperties");
	__vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) vkelGetProcAddr("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	__vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) vkelGetProcAddr("vkGetPhysicalDeviceSurfaceFormatsKHR");
	__vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) vkelGetProcAddr("vkGetPhysicalDeviceSurfacePresentModesKHR");
	__vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR) vkelGetProcAddr("vkGetPhysicalDeviceSurfaceSupportKHR");
	__vkGetPipelineCacheData = (PFN_vkGetPipelineCacheData) vkelGetProcAddr("vkGetPipelineCacheData");
	__vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults) vkelGetProcAddr("vkGetQueryPoolResults");
	__vkGetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity) vkelGetProcAddr("vkGetRenderAreaGranularity");
	__vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) vkelGetProcAddr("vkGetSwapchainImagesKHR");
	__vkInternalAllocationNotification = (PFN_vkInternalAllocationNotification) vkelGetProcAddr("vkInternalAllocationNotification");
	__vkInternalFreeNotification = (PFN_vkInternalFreeNotification) vkelGetProcAddr("vkInternalFreeNotification");
	__vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges) vkelGetProcAddr("vkInvalidateMappedMemoryRanges");
	__vkMapMemory = (PFN_vkMapMemory) vkelGetProcAddr("vkMapMemory");
	__vkMergePipelineCaches = (PFN_vkMergePipelineCaches) vkelGetProcAddr("vkMergePipelineCaches");
	__vkQueueBindSparse = (PFN_vkQueueBindSparse) vkelGetProcAddr("vkQueueBindSparse");
	__vkQueuePresentKHR = (PFN_vkQueuePresentKHR) vkelGetProcAddr("vkQueuePresentKHR");
	__vkQueueSubmit = (PFN_vkQueueSubmit) vkelGetProcAddr("vkQueueSubmit");
	__vkQueueWaitIdle = (PFN_vkQueueWaitIdle) vkelGetProcAddr("vkQueueWaitIdle");
	__vkReallocationFunction = (PFN_vkReallocationFunction) vkelGetProcAddr("vkReallocationFunction");
	__vkResetCommandBuffer = (PFN_vkResetCommandBuffer) vkelGetProcAddr("vkResetCommandBuffer");
	__vkResetCommandPool = (PFN_vkResetCommandPool) vkelGetProcAddr("vkResetCommandPool");
	__vkResetDescriptorPool = (PFN_vkResetDescriptorPool) vkelGetProcAddr("vkResetDescriptorPool");
	__vkResetEvent = (PFN_vkResetEvent) vkelGetProcAddr("vkResetEvent");
	__vkResetFences = (PFN_vkResetFences) vkelGetProcAddr("vkResetFences");
	__vkSetEvent = (PFN_vkSetEvent) vkelGetProcAddr("vkSetEvent");
	__vkUnmapMemory = (PFN_vkUnmapMemory) vkelGetProcAddr("vkUnmapMemory");
	__vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets) vkelGetProcAddr("vkUpdateDescriptorSets");
	__vkVoidFunction = (PFN_vkVoidFunction) vkelGetProcAddr("vkVoidFunction");
	__vkWaitForFences = (PFN_vkWaitForFences) vkelGetProcAddr("vkWaitForFences");

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


	// Instance and device extension names
	VKEL_EXT_debug_report = vkelIsInstanceExtensionSupported(NULL, "VK_EXT_debug_report");
	VKEL_IMG_filter_cubic = vkelIsInstanceExtensionSupported(NULL, "VK_IMG_filter_cubic");
	VKEL_KHR_android_surface = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_android_surface");
	VKEL_KHR_display = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_display");
	VKEL_KHR_display_swapchain = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_display_swapchain");
	VKEL_KHR_mir_surface = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_mir_surface");
	VKEL_KHR_sampler_mirror_clamp_to_edge = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_sampler_mirror_clamp_to_edge");
	VKEL_KHR_surface = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_surface");
	VKEL_KHR_swapchain = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_swapchain");
	VKEL_KHR_wayland_surface = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_wayland_surface");
	VKEL_KHR_win32_surface = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_win32_surface");
	VKEL_KHR_xcb_surface = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_xcb_surface");
	VKEL_KHR_xlib_surface = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_xlib_surface");
	VKEL_NV_glsl_shader = vkelIsInstanceExtensionSupported(NULL, "VK_NV_glsl_shader");

	// Instance and device layer names
	VKEL_LAYER_GOOGLE_unique_objects = vkelIsInstanceLayerSupported("VK_LAYER_GOOGLE_unique_objects");
	VKEL_LAYER_LUNARG_api_dump = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_api_dump");
	VKEL_LAYER_LUNARG_device_limits = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_device_limits");
	VKEL_LAYER_LUNARG_draw_state = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_draw_state");
	VKEL_LAYER_LUNARG_image = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_image");
	VKEL_LAYER_LUNARG_mem_tracker = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_mem_tracker");
	VKEL_LAYER_LUNARG_object_tracker = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_object_tracker");
	VKEL_LAYER_LUNARG_param_checker = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_param_checker");
	VKEL_LAYER_LUNARG_screenshot = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_screenshot");
	VKEL_LAYER_LUNARG_swapchain = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_swapchain");
	VKEL_LAYER_LUNARG_threading = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_threading");
	VKEL_LAYER_LUNARG_vktrace = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_vktrace");

	
	return VK_TRUE;
}
	
VkBool32 vkelInstanceInit(VkInstance instance)
{
	if (!vkelVkLibHandle && !vkelInit())
		return VK_FALSE;
	
	
	__vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR) vkelGetInstanceProcAddr(instance, "vkAcquireNextImageKHR");
	__vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers) vkelGetInstanceProcAddr(instance, "vkAllocateCommandBuffers");
	__vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets) vkelGetInstanceProcAddr(instance, "vkAllocateDescriptorSets");
	__vkAllocateMemory = (PFN_vkAllocateMemory) vkelGetInstanceProcAddr(instance, "vkAllocateMemory");
	__vkAllocationFunction = (PFN_vkAllocationFunction) vkelGetInstanceProcAddr(instance, "vkAllocationFunction");
	__vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer) vkelGetInstanceProcAddr(instance, "vkBeginCommandBuffer");
	__vkBindBufferMemory = (PFN_vkBindBufferMemory) vkelGetInstanceProcAddr(instance, "vkBindBufferMemory");
	__vkBindImageMemory = (PFN_vkBindImageMemory) vkelGetInstanceProcAddr(instance, "vkBindImageMemory");
	__vkCmdBeginQuery = (PFN_vkCmdBeginQuery) vkelGetInstanceProcAddr(instance, "vkCmdBeginQuery");
	__vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass) vkelGetInstanceProcAddr(instance, "vkCmdBeginRenderPass");
	__vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets) vkelGetInstanceProcAddr(instance, "vkCmdBindDescriptorSets");
	__vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer) vkelGetInstanceProcAddr(instance, "vkCmdBindIndexBuffer");
	__vkCmdBindPipeline = (PFN_vkCmdBindPipeline) vkelGetInstanceProcAddr(instance, "vkCmdBindPipeline");
	__vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers) vkelGetInstanceProcAddr(instance, "vkCmdBindVertexBuffers");
	__vkCmdBlitImage = (PFN_vkCmdBlitImage) vkelGetInstanceProcAddr(instance, "vkCmdBlitImage");
	__vkCmdClearAttachments = (PFN_vkCmdClearAttachments) vkelGetInstanceProcAddr(instance, "vkCmdClearAttachments");
	__vkCmdClearColorImage = (PFN_vkCmdClearColorImage) vkelGetInstanceProcAddr(instance, "vkCmdClearColorImage");
	__vkCmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage) vkelGetInstanceProcAddr(instance, "vkCmdClearDepthStencilImage");
	__vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer) vkelGetInstanceProcAddr(instance, "vkCmdCopyBuffer");
	__vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage) vkelGetInstanceProcAddr(instance, "vkCmdCopyBufferToImage");
	__vkCmdCopyImage = (PFN_vkCmdCopyImage) vkelGetInstanceProcAddr(instance, "vkCmdCopyImage");
	__vkCmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer) vkelGetInstanceProcAddr(instance, "vkCmdCopyImageToBuffer");
	__vkCmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults) vkelGetInstanceProcAddr(instance, "vkCmdCopyQueryPoolResults");
	__vkCmdDispatch = (PFN_vkCmdDispatch) vkelGetInstanceProcAddr(instance, "vkCmdDispatch");
	__vkCmdDispatchIndirect = (PFN_vkCmdDispatchIndirect) vkelGetInstanceProcAddr(instance, "vkCmdDispatchIndirect");
	__vkCmdDraw = (PFN_vkCmdDraw) vkelGetInstanceProcAddr(instance, "vkCmdDraw");
	__vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed) vkelGetInstanceProcAddr(instance, "vkCmdDrawIndexed");
	__vkCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect) vkelGetInstanceProcAddr(instance, "vkCmdDrawIndexedIndirect");
	__vkCmdDrawIndirect = (PFN_vkCmdDrawIndirect) vkelGetInstanceProcAddr(instance, "vkCmdDrawIndirect");
	__vkCmdEndQuery = (PFN_vkCmdEndQuery) vkelGetInstanceProcAddr(instance, "vkCmdEndQuery");
	__vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass) vkelGetInstanceProcAddr(instance, "vkCmdEndRenderPass");
	__vkCmdExecuteCommands = (PFN_vkCmdExecuteCommands) vkelGetInstanceProcAddr(instance, "vkCmdExecuteCommands");
	__vkCmdFillBuffer = (PFN_vkCmdFillBuffer) vkelGetInstanceProcAddr(instance, "vkCmdFillBuffer");
	__vkCmdNextSubpass = (PFN_vkCmdNextSubpass) vkelGetInstanceProcAddr(instance, "vkCmdNextSubpass");
	__vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier) vkelGetInstanceProcAddr(instance, "vkCmdPipelineBarrier");
	__vkCmdPushConstants = (PFN_vkCmdPushConstants) vkelGetInstanceProcAddr(instance, "vkCmdPushConstants");
	__vkCmdResetEvent = (PFN_vkCmdResetEvent) vkelGetInstanceProcAddr(instance, "vkCmdResetEvent");
	__vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool) vkelGetInstanceProcAddr(instance, "vkCmdResetQueryPool");
	__vkCmdResolveImage = (PFN_vkCmdResolveImage) vkelGetInstanceProcAddr(instance, "vkCmdResolveImage");
	__vkCmdSetBlendConstants = (PFN_vkCmdSetBlendConstants) vkelGetInstanceProcAddr(instance, "vkCmdSetBlendConstants");
	__vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias) vkelGetInstanceProcAddr(instance, "vkCmdSetDepthBias");
	__vkCmdSetDepthBounds = (PFN_vkCmdSetDepthBounds) vkelGetInstanceProcAddr(instance, "vkCmdSetDepthBounds");
	__vkCmdSetEvent = (PFN_vkCmdSetEvent) vkelGetInstanceProcAddr(instance, "vkCmdSetEvent");
	__vkCmdSetLineWidth = (PFN_vkCmdSetLineWidth) vkelGetInstanceProcAddr(instance, "vkCmdSetLineWidth");
	__vkCmdSetScissor = (PFN_vkCmdSetScissor) vkelGetInstanceProcAddr(instance, "vkCmdSetScissor");
	__vkCmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask) vkelGetInstanceProcAddr(instance, "vkCmdSetStencilCompareMask");
	__vkCmdSetStencilReference = (PFN_vkCmdSetStencilReference) vkelGetInstanceProcAddr(instance, "vkCmdSetStencilReference");
	__vkCmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask) vkelGetInstanceProcAddr(instance, "vkCmdSetStencilWriteMask");
	__vkCmdSetViewport = (PFN_vkCmdSetViewport) vkelGetInstanceProcAddr(instance, "vkCmdSetViewport");
	__vkCmdUpdateBuffer = (PFN_vkCmdUpdateBuffer) vkelGetInstanceProcAddr(instance, "vkCmdUpdateBuffer");
	__vkCmdWaitEvents = (PFN_vkCmdWaitEvents) vkelGetInstanceProcAddr(instance, "vkCmdWaitEvents");
	__vkCmdWriteTimestamp = (PFN_vkCmdWriteTimestamp) vkelGetInstanceProcAddr(instance, "vkCmdWriteTimestamp");
	__vkCreateBuffer = (PFN_vkCreateBuffer) vkelGetInstanceProcAddr(instance, "vkCreateBuffer");
	__vkCreateBufferView = (PFN_vkCreateBufferView) vkelGetInstanceProcAddr(instance, "vkCreateBufferView");
	__vkCreateCommandPool = (PFN_vkCreateCommandPool) vkelGetInstanceProcAddr(instance, "vkCreateCommandPool");
	__vkCreateComputePipelines = (PFN_vkCreateComputePipelines) vkelGetInstanceProcAddr(instance, "vkCreateComputePipelines");
	__vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) vkelGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	__vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool) vkelGetInstanceProcAddr(instance, "vkCreateDescriptorPool");
	__vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout) vkelGetInstanceProcAddr(instance, "vkCreateDescriptorSetLayout");
	__vkCreateDevice = (PFN_vkCreateDevice) vkelGetInstanceProcAddr(instance, "vkCreateDevice");
	__vkCreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR) vkelGetInstanceProcAddr(instance, "vkCreateDisplayModeKHR");
	__vkCreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR) vkelGetInstanceProcAddr(instance, "vkCreateDisplayPlaneSurfaceKHR");
	__vkCreateEvent = (PFN_vkCreateEvent) vkelGetInstanceProcAddr(instance, "vkCreateEvent");
	__vkCreateFence = (PFN_vkCreateFence) vkelGetInstanceProcAddr(instance, "vkCreateFence");
	__vkCreateFramebuffer = (PFN_vkCreateFramebuffer) vkelGetInstanceProcAddr(instance, "vkCreateFramebuffer");
	__vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines) vkelGetInstanceProcAddr(instance, "vkCreateGraphicsPipelines");
	__vkCreateImage = (PFN_vkCreateImage) vkelGetInstanceProcAddr(instance, "vkCreateImage");
	__vkCreateImageView = (PFN_vkCreateImageView) vkelGetInstanceProcAddr(instance, "vkCreateImageView");
	__vkCreateInstance = (PFN_vkCreateInstance) vkelGetInstanceProcAddr(instance, "vkCreateInstance");
	__vkCreatePipelineCache = (PFN_vkCreatePipelineCache) vkelGetInstanceProcAddr(instance, "vkCreatePipelineCache");
	__vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout) vkelGetInstanceProcAddr(instance, "vkCreatePipelineLayout");
	__vkCreateQueryPool = (PFN_vkCreateQueryPool) vkelGetInstanceProcAddr(instance, "vkCreateQueryPool");
	__vkCreateRenderPass = (PFN_vkCreateRenderPass) vkelGetInstanceProcAddr(instance, "vkCreateRenderPass");
	__vkCreateSampler = (PFN_vkCreateSampler) vkelGetInstanceProcAddr(instance, "vkCreateSampler");
	__vkCreateSemaphore = (PFN_vkCreateSemaphore) vkelGetInstanceProcAddr(instance, "vkCreateSemaphore");
	__vkCreateShaderModule = (PFN_vkCreateShaderModule) vkelGetInstanceProcAddr(instance, "vkCreateShaderModule");
	__vkCreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR) vkelGetInstanceProcAddr(instance, "vkCreateSharedSwapchainsKHR");
	__vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR) vkelGetInstanceProcAddr(instance, "vkCreateSwapchainKHR");
	__vkDebugReportCallbackEXT = (PFN_vkDebugReportCallbackEXT) vkelGetInstanceProcAddr(instance, "vkDebugReportCallbackEXT");
	__vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT) vkelGetInstanceProcAddr(instance, "vkDebugReportMessageEXT");
	__vkDestroyBuffer = (PFN_vkDestroyBuffer) vkelGetInstanceProcAddr(instance, "vkDestroyBuffer");
	__vkDestroyBufferView = (PFN_vkDestroyBufferView) vkelGetInstanceProcAddr(instance, "vkDestroyBufferView");
	__vkDestroyCommandPool = (PFN_vkDestroyCommandPool) vkelGetInstanceProcAddr(instance, "vkDestroyCommandPool");
	__vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) vkelGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	__vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool) vkelGetInstanceProcAddr(instance, "vkDestroyDescriptorPool");
	__vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout) vkelGetInstanceProcAddr(instance, "vkDestroyDescriptorSetLayout");
	__vkDestroyDevice = (PFN_vkDestroyDevice) vkelGetInstanceProcAddr(instance, "vkDestroyDevice");
	__vkDestroyEvent = (PFN_vkDestroyEvent) vkelGetInstanceProcAddr(instance, "vkDestroyEvent");
	__vkDestroyFence = (PFN_vkDestroyFence) vkelGetInstanceProcAddr(instance, "vkDestroyFence");
	__vkDestroyFramebuffer = (PFN_vkDestroyFramebuffer) vkelGetInstanceProcAddr(instance, "vkDestroyFramebuffer");
	__vkDestroyImage = (PFN_vkDestroyImage) vkelGetInstanceProcAddr(instance, "vkDestroyImage");
	__vkDestroyImageView = (PFN_vkDestroyImageView) vkelGetInstanceProcAddr(instance, "vkDestroyImageView");
	__vkDestroyInstance = (PFN_vkDestroyInstance) vkelGetInstanceProcAddr(instance, "vkDestroyInstance");
	__vkDestroyPipeline = (PFN_vkDestroyPipeline) vkelGetInstanceProcAddr(instance, "vkDestroyPipeline");
	__vkDestroyPipelineCache = (PFN_vkDestroyPipelineCache) vkelGetInstanceProcAddr(instance, "vkDestroyPipelineCache");
	__vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout) vkelGetInstanceProcAddr(instance, "vkDestroyPipelineLayout");
	__vkDestroyQueryPool = (PFN_vkDestroyQueryPool) vkelGetInstanceProcAddr(instance, "vkDestroyQueryPool");
	__vkDestroyRenderPass = (PFN_vkDestroyRenderPass) vkelGetInstanceProcAddr(instance, "vkDestroyRenderPass");
	__vkDestroySampler = (PFN_vkDestroySampler) vkelGetInstanceProcAddr(instance, "vkDestroySampler");
	__vkDestroySemaphore = (PFN_vkDestroySemaphore) vkelGetInstanceProcAddr(instance, "vkDestroySemaphore");
	__vkDestroyShaderModule = (PFN_vkDestroyShaderModule) vkelGetInstanceProcAddr(instance, "vkDestroyShaderModule");
	__vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR) vkelGetInstanceProcAddr(instance, "vkDestroySurfaceKHR");
	__vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR) vkelGetInstanceProcAddr(instance, "vkDestroySwapchainKHR");
	__vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle) vkelGetInstanceProcAddr(instance, "vkDeviceWaitIdle");
	__vkEndCommandBuffer = (PFN_vkEndCommandBuffer) vkelGetInstanceProcAddr(instance, "vkEndCommandBuffer");
	__vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties) vkelGetInstanceProcAddr(instance, "vkEnumerateDeviceExtensionProperties");
	__vkEnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties) vkelGetInstanceProcAddr(instance, "vkEnumerateDeviceLayerProperties");
	__vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties) vkelGetInstanceProcAddr(instance, "vkEnumerateInstanceExtensionProperties");
	__vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties) vkelGetInstanceProcAddr(instance, "vkEnumerateInstanceLayerProperties");
	__vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) vkelGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices");
	__vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges) vkelGetInstanceProcAddr(instance, "vkFlushMappedMemoryRanges");
	__vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers) vkelGetInstanceProcAddr(instance, "vkFreeCommandBuffers");
	__vkFreeDescriptorSets = (PFN_vkFreeDescriptorSets) vkelGetInstanceProcAddr(instance, "vkFreeDescriptorSets");
	__vkFreeFunction = (PFN_vkFreeFunction) vkelGetInstanceProcAddr(instance, "vkFreeFunction");
	__vkFreeMemory = (PFN_vkFreeMemory) vkelGetInstanceProcAddr(instance, "vkFreeMemory");
	__vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements) vkelGetInstanceProcAddr(instance, "vkGetBufferMemoryRequirements");
	__vkGetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment) vkelGetInstanceProcAddr(instance, "vkGetDeviceMemoryCommitment");
	__vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr) vkelGetInstanceProcAddr(instance, "vkGetDeviceProcAddr");
	__vkGetDeviceQueue = (PFN_vkGetDeviceQueue) vkelGetInstanceProcAddr(instance, "vkGetDeviceQueue");
	__vkGetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR) vkelGetInstanceProcAddr(instance, "vkGetDisplayModePropertiesKHR");
	__vkGetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR) vkelGetInstanceProcAddr(instance, "vkGetDisplayPlaneCapabilitiesKHR");
	__vkGetDisplayPlaneSupportedDisplaysKHR = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR) vkelGetInstanceProcAddr(instance, "vkGetDisplayPlaneSupportedDisplaysKHR");
	__vkGetEventStatus = (PFN_vkGetEventStatus) vkelGetInstanceProcAddr(instance, "vkGetEventStatus");
	__vkGetFenceStatus = (PFN_vkGetFenceStatus) vkelGetInstanceProcAddr(instance, "vkGetFenceStatus");
	__vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements) vkelGetInstanceProcAddr(instance, "vkGetImageMemoryRequirements");
	__vkGetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements) vkelGetInstanceProcAddr(instance, "vkGetImageSparseMemoryRequirements");
	__vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout) vkelGetInstanceProcAddr(instance, "vkGetImageSubresourceLayout");
	__vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) vkelGetInstanceProcAddr(instance, "vkGetInstanceProcAddr");
	__vkGetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
	__vkGetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");
	__vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures");
	__vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFormatProperties");
	__vkGetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceImageFormatProperties");
	__vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties");
	__vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties");
	__vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties");
	__vkGetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSparseImageFormatProperties");
	__vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	__vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
	__vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
	__vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
	__vkGetPipelineCacheData = (PFN_vkGetPipelineCacheData) vkelGetInstanceProcAddr(instance, "vkGetPipelineCacheData");
	__vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults) vkelGetInstanceProcAddr(instance, "vkGetQueryPoolResults");
	__vkGetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity) vkelGetInstanceProcAddr(instance, "vkGetRenderAreaGranularity");
	__vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) vkelGetInstanceProcAddr(instance, "vkGetSwapchainImagesKHR");
	__vkInternalAllocationNotification = (PFN_vkInternalAllocationNotification) vkelGetInstanceProcAddr(instance, "vkInternalAllocationNotification");
	__vkInternalFreeNotification = (PFN_vkInternalFreeNotification) vkelGetInstanceProcAddr(instance, "vkInternalFreeNotification");
	__vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges) vkelGetInstanceProcAddr(instance, "vkInvalidateMappedMemoryRanges");
	__vkMapMemory = (PFN_vkMapMemory) vkelGetInstanceProcAddr(instance, "vkMapMemory");
	__vkMergePipelineCaches = (PFN_vkMergePipelineCaches) vkelGetInstanceProcAddr(instance, "vkMergePipelineCaches");
	__vkQueueBindSparse = (PFN_vkQueueBindSparse) vkelGetInstanceProcAddr(instance, "vkQueueBindSparse");
	__vkQueuePresentKHR = (PFN_vkQueuePresentKHR) vkelGetInstanceProcAddr(instance, "vkQueuePresentKHR");
	__vkQueueSubmit = (PFN_vkQueueSubmit) vkelGetInstanceProcAddr(instance, "vkQueueSubmit");
	__vkQueueWaitIdle = (PFN_vkQueueWaitIdle) vkelGetInstanceProcAddr(instance, "vkQueueWaitIdle");
	__vkReallocationFunction = (PFN_vkReallocationFunction) vkelGetInstanceProcAddr(instance, "vkReallocationFunction");
	__vkResetCommandBuffer = (PFN_vkResetCommandBuffer) vkelGetInstanceProcAddr(instance, "vkResetCommandBuffer");
	__vkResetCommandPool = (PFN_vkResetCommandPool) vkelGetInstanceProcAddr(instance, "vkResetCommandPool");
	__vkResetDescriptorPool = (PFN_vkResetDescriptorPool) vkelGetInstanceProcAddr(instance, "vkResetDescriptorPool");
	__vkResetEvent = (PFN_vkResetEvent) vkelGetInstanceProcAddr(instance, "vkResetEvent");
	__vkResetFences = (PFN_vkResetFences) vkelGetInstanceProcAddr(instance, "vkResetFences");
	__vkSetEvent = (PFN_vkSetEvent) vkelGetInstanceProcAddr(instance, "vkSetEvent");
	__vkUnmapMemory = (PFN_vkUnmapMemory) vkelGetInstanceProcAddr(instance, "vkUnmapMemory");
	__vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets) vkelGetInstanceProcAddr(instance, "vkUpdateDescriptorSets");
	__vkVoidFunction = (PFN_vkVoidFunction) vkelGetInstanceProcAddr(instance, "vkVoidFunction");
	__vkWaitForFences = (PFN_vkWaitForFences) vkelGetInstanceProcAddr(instance, "vkWaitForFences");

#ifdef VK_USE_PLATFORM_ANDROID_KHR
	__vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR) vkelGetInstanceProcAddr(instance, "vkCreateAndroidSurfaceKHR");
#endif /* VK_USE_PLATFORM_ANDROID_KHR */

#ifdef VK_USE_PLATFORM_MIR_KHR
	__vkCreateMirSurfaceKHR = (PFN_vkCreateMirSurfaceKHR) vkelGetInstanceProcAddr(instance, "vkCreateMirSurfaceKHR");
	__vkGetPhysicalDeviceMirPresentationSupportKHR = (PFN_vkGetPhysicalDeviceMirPresentationSupportKHR) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMirPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_MIR_KHR */

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	__vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR) vkelGetInstanceProcAddr(instance, "vkCreateWaylandSurfaceKHR");
	__vkGetPhysicalDeviceWaylandPresentationSupportKHR = (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_WAYLAND_KHR */

#ifdef VK_USE_PLATFORM_WIN32_KHR
	__vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR) vkelGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
	__vkGetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif /* VK_USE_PLATFORM_WIN32_KHR */

#ifdef VK_USE_PLATFORM_XCB_KHR
	__vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR) vkelGetInstanceProcAddr(instance, "vkCreateXcbSurfaceKHR");
	__vkGetPhysicalDeviceXcbPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_XCB_KHR */

#ifdef VK_USE_PLATFORM_XLIB_KHR
	__vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR) vkelGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
	__vkGetPhysicalDeviceXlibPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR) vkelGetInstanceProcAddr(instance, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_XLIB_KHR */


	// Instance and device extension names
	VKEL_EXT_debug_report = vkelIsInstanceExtensionSupported(NULL, "VK_EXT_debug_report");
	VKEL_IMG_filter_cubic = vkelIsInstanceExtensionSupported(NULL, "VK_IMG_filter_cubic");
	VKEL_KHR_android_surface = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_android_surface");
	VKEL_KHR_display = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_display");
	VKEL_KHR_display_swapchain = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_display_swapchain");
	VKEL_KHR_mir_surface = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_mir_surface");
	VKEL_KHR_sampler_mirror_clamp_to_edge = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_sampler_mirror_clamp_to_edge");
	VKEL_KHR_surface = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_surface");
	VKEL_KHR_swapchain = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_swapchain");
	VKEL_KHR_wayland_surface = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_wayland_surface");
	VKEL_KHR_win32_surface = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_win32_surface");
	VKEL_KHR_xcb_surface = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_xcb_surface");
	VKEL_KHR_xlib_surface = vkelIsInstanceExtensionSupported(NULL, "VK_KHR_xlib_surface");
	VKEL_NV_glsl_shader = vkelIsInstanceExtensionSupported(NULL, "VK_NV_glsl_shader");

	// Instance and device layer names
	VKEL_LAYER_GOOGLE_unique_objects = vkelIsInstanceLayerSupported("VK_LAYER_GOOGLE_unique_objects");
	VKEL_LAYER_LUNARG_api_dump = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_api_dump");
	VKEL_LAYER_LUNARG_device_limits = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_device_limits");
	VKEL_LAYER_LUNARG_draw_state = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_draw_state");
	VKEL_LAYER_LUNARG_image = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_image");
	VKEL_LAYER_LUNARG_mem_tracker = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_mem_tracker");
	VKEL_LAYER_LUNARG_object_tracker = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_object_tracker");
	VKEL_LAYER_LUNARG_param_checker = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_param_checker");
	VKEL_LAYER_LUNARG_screenshot = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_screenshot");
	VKEL_LAYER_LUNARG_swapchain = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_swapchain");
	VKEL_LAYER_LUNARG_threading = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_threading");
	VKEL_LAYER_LUNARG_vktrace = vkelIsInstanceLayerSupported("VK_LAYER_LUNARG_vktrace");

	
	return VK_TRUE;
}
	
VkBool32 vkelDeviceInit(VkPhysicalDevice physicalDevice, VkDevice device)
{
	if (!vkelVkLibHandle && !vkelInit())
		return VK_FALSE;
	
	
	__vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR) vkelGetDeviceProcAddr(device, "vkAcquireNextImageKHR");
	__vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers) vkelGetDeviceProcAddr(device, "vkAllocateCommandBuffers");
	__vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets) vkelGetDeviceProcAddr(device, "vkAllocateDescriptorSets");
	__vkAllocateMemory = (PFN_vkAllocateMemory) vkelGetDeviceProcAddr(device, "vkAllocateMemory");
	__vkAllocationFunction = (PFN_vkAllocationFunction) vkelGetDeviceProcAddr(device, "vkAllocationFunction");
	__vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer) vkelGetDeviceProcAddr(device, "vkBeginCommandBuffer");
	__vkBindBufferMemory = (PFN_vkBindBufferMemory) vkelGetDeviceProcAddr(device, "vkBindBufferMemory");
	__vkBindImageMemory = (PFN_vkBindImageMemory) vkelGetDeviceProcAddr(device, "vkBindImageMemory");
	__vkCmdBeginQuery = (PFN_vkCmdBeginQuery) vkelGetDeviceProcAddr(device, "vkCmdBeginQuery");
	__vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass) vkelGetDeviceProcAddr(device, "vkCmdBeginRenderPass");
	__vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets) vkelGetDeviceProcAddr(device, "vkCmdBindDescriptorSets");
	__vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer) vkelGetDeviceProcAddr(device, "vkCmdBindIndexBuffer");
	__vkCmdBindPipeline = (PFN_vkCmdBindPipeline) vkelGetDeviceProcAddr(device, "vkCmdBindPipeline");
	__vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers) vkelGetDeviceProcAddr(device, "vkCmdBindVertexBuffers");
	__vkCmdBlitImage = (PFN_vkCmdBlitImage) vkelGetDeviceProcAddr(device, "vkCmdBlitImage");
	__vkCmdClearAttachments = (PFN_vkCmdClearAttachments) vkelGetDeviceProcAddr(device, "vkCmdClearAttachments");
	__vkCmdClearColorImage = (PFN_vkCmdClearColorImage) vkelGetDeviceProcAddr(device, "vkCmdClearColorImage");
	__vkCmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage) vkelGetDeviceProcAddr(device, "vkCmdClearDepthStencilImage");
	__vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer) vkelGetDeviceProcAddr(device, "vkCmdCopyBuffer");
	__vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage) vkelGetDeviceProcAddr(device, "vkCmdCopyBufferToImage");
	__vkCmdCopyImage = (PFN_vkCmdCopyImage) vkelGetDeviceProcAddr(device, "vkCmdCopyImage");
	__vkCmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer) vkelGetDeviceProcAddr(device, "vkCmdCopyImageToBuffer");
	__vkCmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults) vkelGetDeviceProcAddr(device, "vkCmdCopyQueryPoolResults");
	__vkCmdDispatch = (PFN_vkCmdDispatch) vkelGetDeviceProcAddr(device, "vkCmdDispatch");
	__vkCmdDispatchIndirect = (PFN_vkCmdDispatchIndirect) vkelGetDeviceProcAddr(device, "vkCmdDispatchIndirect");
	__vkCmdDraw = (PFN_vkCmdDraw) vkelGetDeviceProcAddr(device, "vkCmdDraw");
	__vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed) vkelGetDeviceProcAddr(device, "vkCmdDrawIndexed");
	__vkCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect) vkelGetDeviceProcAddr(device, "vkCmdDrawIndexedIndirect");
	__vkCmdDrawIndirect = (PFN_vkCmdDrawIndirect) vkelGetDeviceProcAddr(device, "vkCmdDrawIndirect");
	__vkCmdEndQuery = (PFN_vkCmdEndQuery) vkelGetDeviceProcAddr(device, "vkCmdEndQuery");
	__vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass) vkelGetDeviceProcAddr(device, "vkCmdEndRenderPass");
	__vkCmdExecuteCommands = (PFN_vkCmdExecuteCommands) vkelGetDeviceProcAddr(device, "vkCmdExecuteCommands");
	__vkCmdFillBuffer = (PFN_vkCmdFillBuffer) vkelGetDeviceProcAddr(device, "vkCmdFillBuffer");
	__vkCmdNextSubpass = (PFN_vkCmdNextSubpass) vkelGetDeviceProcAddr(device, "vkCmdNextSubpass");
	__vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier) vkelGetDeviceProcAddr(device, "vkCmdPipelineBarrier");
	__vkCmdPushConstants = (PFN_vkCmdPushConstants) vkelGetDeviceProcAddr(device, "vkCmdPushConstants");
	__vkCmdResetEvent = (PFN_vkCmdResetEvent) vkelGetDeviceProcAddr(device, "vkCmdResetEvent");
	__vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool) vkelGetDeviceProcAddr(device, "vkCmdResetQueryPool");
	__vkCmdResolveImage = (PFN_vkCmdResolveImage) vkelGetDeviceProcAddr(device, "vkCmdResolveImage");
	__vkCmdSetBlendConstants = (PFN_vkCmdSetBlendConstants) vkelGetDeviceProcAddr(device, "vkCmdSetBlendConstants");
	__vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias) vkelGetDeviceProcAddr(device, "vkCmdSetDepthBias");
	__vkCmdSetDepthBounds = (PFN_vkCmdSetDepthBounds) vkelGetDeviceProcAddr(device, "vkCmdSetDepthBounds");
	__vkCmdSetEvent = (PFN_vkCmdSetEvent) vkelGetDeviceProcAddr(device, "vkCmdSetEvent");
	__vkCmdSetLineWidth = (PFN_vkCmdSetLineWidth) vkelGetDeviceProcAddr(device, "vkCmdSetLineWidth");
	__vkCmdSetScissor = (PFN_vkCmdSetScissor) vkelGetDeviceProcAddr(device, "vkCmdSetScissor");
	__vkCmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask) vkelGetDeviceProcAddr(device, "vkCmdSetStencilCompareMask");
	__vkCmdSetStencilReference = (PFN_vkCmdSetStencilReference) vkelGetDeviceProcAddr(device, "vkCmdSetStencilReference");
	__vkCmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask) vkelGetDeviceProcAddr(device, "vkCmdSetStencilWriteMask");
	__vkCmdSetViewport = (PFN_vkCmdSetViewport) vkelGetDeviceProcAddr(device, "vkCmdSetViewport");
	__vkCmdUpdateBuffer = (PFN_vkCmdUpdateBuffer) vkelGetDeviceProcAddr(device, "vkCmdUpdateBuffer");
	__vkCmdWaitEvents = (PFN_vkCmdWaitEvents) vkelGetDeviceProcAddr(device, "vkCmdWaitEvents");
	__vkCmdWriteTimestamp = (PFN_vkCmdWriteTimestamp) vkelGetDeviceProcAddr(device, "vkCmdWriteTimestamp");
	__vkCreateBuffer = (PFN_vkCreateBuffer) vkelGetDeviceProcAddr(device, "vkCreateBuffer");
	__vkCreateBufferView = (PFN_vkCreateBufferView) vkelGetDeviceProcAddr(device, "vkCreateBufferView");
	__vkCreateCommandPool = (PFN_vkCreateCommandPool) vkelGetDeviceProcAddr(device, "vkCreateCommandPool");
	__vkCreateComputePipelines = (PFN_vkCreateComputePipelines) vkelGetDeviceProcAddr(device, "vkCreateComputePipelines");
	__vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) vkelGetDeviceProcAddr(device, "vkCreateDebugReportCallbackEXT");
	__vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool) vkelGetDeviceProcAddr(device, "vkCreateDescriptorPool");
	__vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout) vkelGetDeviceProcAddr(device, "vkCreateDescriptorSetLayout");
	__vkCreateDevice = (PFN_vkCreateDevice) vkelGetDeviceProcAddr(device, "vkCreateDevice");
	__vkCreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR) vkelGetDeviceProcAddr(device, "vkCreateDisplayModeKHR");
	__vkCreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR) vkelGetDeviceProcAddr(device, "vkCreateDisplayPlaneSurfaceKHR");
	__vkCreateEvent = (PFN_vkCreateEvent) vkelGetDeviceProcAddr(device, "vkCreateEvent");
	__vkCreateFence = (PFN_vkCreateFence) vkelGetDeviceProcAddr(device, "vkCreateFence");
	__vkCreateFramebuffer = (PFN_vkCreateFramebuffer) vkelGetDeviceProcAddr(device, "vkCreateFramebuffer");
	__vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines) vkelGetDeviceProcAddr(device, "vkCreateGraphicsPipelines");
	__vkCreateImage = (PFN_vkCreateImage) vkelGetDeviceProcAddr(device, "vkCreateImage");
	__vkCreateImageView = (PFN_vkCreateImageView) vkelGetDeviceProcAddr(device, "vkCreateImageView");
	__vkCreateInstance = (PFN_vkCreateInstance) vkelGetDeviceProcAddr(device, "vkCreateInstance");
	__vkCreatePipelineCache = (PFN_vkCreatePipelineCache) vkelGetDeviceProcAddr(device, "vkCreatePipelineCache");
	__vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout) vkelGetDeviceProcAddr(device, "vkCreatePipelineLayout");
	__vkCreateQueryPool = (PFN_vkCreateQueryPool) vkelGetDeviceProcAddr(device, "vkCreateQueryPool");
	__vkCreateRenderPass = (PFN_vkCreateRenderPass) vkelGetDeviceProcAddr(device, "vkCreateRenderPass");
	__vkCreateSampler = (PFN_vkCreateSampler) vkelGetDeviceProcAddr(device, "vkCreateSampler");
	__vkCreateSemaphore = (PFN_vkCreateSemaphore) vkelGetDeviceProcAddr(device, "vkCreateSemaphore");
	__vkCreateShaderModule = (PFN_vkCreateShaderModule) vkelGetDeviceProcAddr(device, "vkCreateShaderModule");
	__vkCreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR) vkelGetDeviceProcAddr(device, "vkCreateSharedSwapchainsKHR");
	__vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR) vkelGetDeviceProcAddr(device, "vkCreateSwapchainKHR");
	__vkDebugReportCallbackEXT = (PFN_vkDebugReportCallbackEXT) vkelGetDeviceProcAddr(device, "vkDebugReportCallbackEXT");
	__vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT) vkelGetDeviceProcAddr(device, "vkDebugReportMessageEXT");
	__vkDestroyBuffer = (PFN_vkDestroyBuffer) vkelGetDeviceProcAddr(device, "vkDestroyBuffer");
	__vkDestroyBufferView = (PFN_vkDestroyBufferView) vkelGetDeviceProcAddr(device, "vkDestroyBufferView");
	__vkDestroyCommandPool = (PFN_vkDestroyCommandPool) vkelGetDeviceProcAddr(device, "vkDestroyCommandPool");
	__vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) vkelGetDeviceProcAddr(device, "vkDestroyDebugReportCallbackEXT");
	__vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool) vkelGetDeviceProcAddr(device, "vkDestroyDescriptorPool");
	__vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout) vkelGetDeviceProcAddr(device, "vkDestroyDescriptorSetLayout");
	__vkDestroyDevice = (PFN_vkDestroyDevice) vkelGetDeviceProcAddr(device, "vkDestroyDevice");
	__vkDestroyEvent = (PFN_vkDestroyEvent) vkelGetDeviceProcAddr(device, "vkDestroyEvent");
	__vkDestroyFence = (PFN_vkDestroyFence) vkelGetDeviceProcAddr(device, "vkDestroyFence");
	__vkDestroyFramebuffer = (PFN_vkDestroyFramebuffer) vkelGetDeviceProcAddr(device, "vkDestroyFramebuffer");
	__vkDestroyImage = (PFN_vkDestroyImage) vkelGetDeviceProcAddr(device, "vkDestroyImage");
	__vkDestroyImageView = (PFN_vkDestroyImageView) vkelGetDeviceProcAddr(device, "vkDestroyImageView");
	__vkDestroyInstance = (PFN_vkDestroyInstance) vkelGetDeviceProcAddr(device, "vkDestroyInstance");
	__vkDestroyPipeline = (PFN_vkDestroyPipeline) vkelGetDeviceProcAddr(device, "vkDestroyPipeline");
	__vkDestroyPipelineCache = (PFN_vkDestroyPipelineCache) vkelGetDeviceProcAddr(device, "vkDestroyPipelineCache");
	__vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout) vkelGetDeviceProcAddr(device, "vkDestroyPipelineLayout");
	__vkDestroyQueryPool = (PFN_vkDestroyQueryPool) vkelGetDeviceProcAddr(device, "vkDestroyQueryPool");
	__vkDestroyRenderPass = (PFN_vkDestroyRenderPass) vkelGetDeviceProcAddr(device, "vkDestroyRenderPass");
	__vkDestroySampler = (PFN_vkDestroySampler) vkelGetDeviceProcAddr(device, "vkDestroySampler");
	__vkDestroySemaphore = (PFN_vkDestroySemaphore) vkelGetDeviceProcAddr(device, "vkDestroySemaphore");
	__vkDestroyShaderModule = (PFN_vkDestroyShaderModule) vkelGetDeviceProcAddr(device, "vkDestroyShaderModule");
	__vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR) vkelGetDeviceProcAddr(device, "vkDestroySurfaceKHR");
	__vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR) vkelGetDeviceProcAddr(device, "vkDestroySwapchainKHR");
	__vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle) vkelGetDeviceProcAddr(device, "vkDeviceWaitIdle");
	__vkEndCommandBuffer = (PFN_vkEndCommandBuffer) vkelGetDeviceProcAddr(device, "vkEndCommandBuffer");
	__vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties) vkelGetDeviceProcAddr(device, "vkEnumerateDeviceExtensionProperties");
	__vkEnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties) vkelGetDeviceProcAddr(device, "vkEnumerateDeviceLayerProperties");
	__vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties) vkelGetDeviceProcAddr(device, "vkEnumerateInstanceExtensionProperties");
	__vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties) vkelGetDeviceProcAddr(device, "vkEnumerateInstanceLayerProperties");
	__vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) vkelGetDeviceProcAddr(device, "vkEnumeratePhysicalDevices");
	__vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges) vkelGetDeviceProcAddr(device, "vkFlushMappedMemoryRanges");
	__vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers) vkelGetDeviceProcAddr(device, "vkFreeCommandBuffers");
	__vkFreeDescriptorSets = (PFN_vkFreeDescriptorSets) vkelGetDeviceProcAddr(device, "vkFreeDescriptorSets");
	__vkFreeFunction = (PFN_vkFreeFunction) vkelGetDeviceProcAddr(device, "vkFreeFunction");
	__vkFreeMemory = (PFN_vkFreeMemory) vkelGetDeviceProcAddr(device, "vkFreeMemory");
	__vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements) vkelGetDeviceProcAddr(device, "vkGetBufferMemoryRequirements");
	__vkGetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment) vkelGetDeviceProcAddr(device, "vkGetDeviceMemoryCommitment");
	__vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr) vkelGetDeviceProcAddr(device, "vkGetDeviceProcAddr");
	__vkGetDeviceQueue = (PFN_vkGetDeviceQueue) vkelGetDeviceProcAddr(device, "vkGetDeviceQueue");
	__vkGetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR) vkelGetDeviceProcAddr(device, "vkGetDisplayModePropertiesKHR");
	__vkGetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR) vkelGetDeviceProcAddr(device, "vkGetDisplayPlaneCapabilitiesKHR");
	__vkGetDisplayPlaneSupportedDisplaysKHR = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR) vkelGetDeviceProcAddr(device, "vkGetDisplayPlaneSupportedDisplaysKHR");
	__vkGetEventStatus = (PFN_vkGetEventStatus) vkelGetDeviceProcAddr(device, "vkGetEventStatus");
	__vkGetFenceStatus = (PFN_vkGetFenceStatus) vkelGetDeviceProcAddr(device, "vkGetFenceStatus");
	__vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements) vkelGetDeviceProcAddr(device, "vkGetImageMemoryRequirements");
	__vkGetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements) vkelGetDeviceProcAddr(device, "vkGetImageSparseMemoryRequirements");
	__vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout) vkelGetDeviceProcAddr(device, "vkGetImageSubresourceLayout");
	__vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) vkelGetDeviceProcAddr(device, "vkGetInstanceProcAddr");
	__vkGetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
	__vkGetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceDisplayPropertiesKHR");
	__vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceFeatures");
	__vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceFormatProperties");
	__vkGetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceImageFormatProperties");
	__vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceMemoryProperties");
	__vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceProperties");
	__vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceQueueFamilyProperties");
	__vkGetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceSparseImageFormatProperties");
	__vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	__vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceSurfaceFormatsKHR");
	__vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceSurfacePresentModesKHR");
	__vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceSurfaceSupportKHR");
	__vkGetPipelineCacheData = (PFN_vkGetPipelineCacheData) vkelGetDeviceProcAddr(device, "vkGetPipelineCacheData");
	__vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults) vkelGetDeviceProcAddr(device, "vkGetQueryPoolResults");
	__vkGetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity) vkelGetDeviceProcAddr(device, "vkGetRenderAreaGranularity");
	__vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) vkelGetDeviceProcAddr(device, "vkGetSwapchainImagesKHR");
	__vkInternalAllocationNotification = (PFN_vkInternalAllocationNotification) vkelGetDeviceProcAddr(device, "vkInternalAllocationNotification");
	__vkInternalFreeNotification = (PFN_vkInternalFreeNotification) vkelGetDeviceProcAddr(device, "vkInternalFreeNotification");
	__vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges) vkelGetDeviceProcAddr(device, "vkInvalidateMappedMemoryRanges");
	__vkMapMemory = (PFN_vkMapMemory) vkelGetDeviceProcAddr(device, "vkMapMemory");
	__vkMergePipelineCaches = (PFN_vkMergePipelineCaches) vkelGetDeviceProcAddr(device, "vkMergePipelineCaches");
	__vkQueueBindSparse = (PFN_vkQueueBindSparse) vkelGetDeviceProcAddr(device, "vkQueueBindSparse");
	__vkQueuePresentKHR = (PFN_vkQueuePresentKHR) vkelGetDeviceProcAddr(device, "vkQueuePresentKHR");
	__vkQueueSubmit = (PFN_vkQueueSubmit) vkelGetDeviceProcAddr(device, "vkQueueSubmit");
	__vkQueueWaitIdle = (PFN_vkQueueWaitIdle) vkelGetDeviceProcAddr(device, "vkQueueWaitIdle");
	__vkReallocationFunction = (PFN_vkReallocationFunction) vkelGetDeviceProcAddr(device, "vkReallocationFunction");
	__vkResetCommandBuffer = (PFN_vkResetCommandBuffer) vkelGetDeviceProcAddr(device, "vkResetCommandBuffer");
	__vkResetCommandPool = (PFN_vkResetCommandPool) vkelGetDeviceProcAddr(device, "vkResetCommandPool");
	__vkResetDescriptorPool = (PFN_vkResetDescriptorPool) vkelGetDeviceProcAddr(device, "vkResetDescriptorPool");
	__vkResetEvent = (PFN_vkResetEvent) vkelGetDeviceProcAddr(device, "vkResetEvent");
	__vkResetFences = (PFN_vkResetFences) vkelGetDeviceProcAddr(device, "vkResetFences");
	__vkSetEvent = (PFN_vkSetEvent) vkelGetDeviceProcAddr(device, "vkSetEvent");
	__vkUnmapMemory = (PFN_vkUnmapMemory) vkelGetDeviceProcAddr(device, "vkUnmapMemory");
	__vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets) vkelGetDeviceProcAddr(device, "vkUpdateDescriptorSets");
	__vkVoidFunction = (PFN_vkVoidFunction) vkelGetDeviceProcAddr(device, "vkVoidFunction");
	__vkWaitForFences = (PFN_vkWaitForFences) vkelGetDeviceProcAddr(device, "vkWaitForFences");

#ifdef VK_USE_PLATFORM_ANDROID_KHR
	__vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR) vkelGetDeviceProcAddr(device, "vkCreateAndroidSurfaceKHR");
#endif /* VK_USE_PLATFORM_ANDROID_KHR */

#ifdef VK_USE_PLATFORM_MIR_KHR
	__vkCreateMirSurfaceKHR = (PFN_vkCreateMirSurfaceKHR) vkelGetDeviceProcAddr(device, "vkCreateMirSurfaceKHR");
	__vkGetPhysicalDeviceMirPresentationSupportKHR = (PFN_vkGetPhysicalDeviceMirPresentationSupportKHR) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceMirPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_MIR_KHR */

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	__vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR) vkelGetDeviceProcAddr(device, "vkCreateWaylandSurfaceKHR");
	__vkGetPhysicalDeviceWaylandPresentationSupportKHR = (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_WAYLAND_KHR */

#ifdef VK_USE_PLATFORM_WIN32_KHR
	__vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR) vkelGetDeviceProcAddr(device, "vkCreateWin32SurfaceKHR");
	__vkGetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif /* VK_USE_PLATFORM_WIN32_KHR */

#ifdef VK_USE_PLATFORM_XCB_KHR
	__vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR) vkelGetDeviceProcAddr(device, "vkCreateXcbSurfaceKHR");
	__vkGetPhysicalDeviceXcbPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_XCB_KHR */

#ifdef VK_USE_PLATFORM_XLIB_KHR
	__vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR) vkelGetDeviceProcAddr(device, "vkCreateXlibSurfaceKHR");
	__vkGetPhysicalDeviceXlibPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR) vkelGetDeviceProcAddr(device, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_XLIB_KHR */


	// Instance and device extension names
	VKEL_EXT_debug_report = vkelIsDeviceExtensionSupported(physicalDevice, NULL, "VK_EXT_debug_report");
	VKEL_IMG_filter_cubic = vkelIsDeviceExtensionSupported(physicalDevice, NULL, "VK_IMG_filter_cubic");
	VKEL_KHR_android_surface = vkelIsDeviceExtensionSupported(physicalDevice, NULL, "VK_KHR_android_surface");
	VKEL_KHR_display = vkelIsDeviceExtensionSupported(physicalDevice, NULL, "VK_KHR_display");
	VKEL_KHR_display_swapchain = vkelIsDeviceExtensionSupported(physicalDevice, NULL, "VK_KHR_display_swapchain");
	VKEL_KHR_mir_surface = vkelIsDeviceExtensionSupported(physicalDevice, NULL, "VK_KHR_mir_surface");
	VKEL_KHR_sampler_mirror_clamp_to_edge = vkelIsDeviceExtensionSupported(physicalDevice, NULL, "VK_KHR_sampler_mirror_clamp_to_edge");
	VKEL_KHR_surface = vkelIsDeviceExtensionSupported(physicalDevice, NULL, "VK_KHR_surface");
	VKEL_KHR_swapchain = vkelIsDeviceExtensionSupported(physicalDevice, NULL, "VK_KHR_swapchain");
	VKEL_KHR_wayland_surface = vkelIsDeviceExtensionSupported(physicalDevice, NULL, "VK_KHR_wayland_surface");
	VKEL_KHR_win32_surface = vkelIsDeviceExtensionSupported(physicalDevice, NULL, "VK_KHR_win32_surface");
	VKEL_KHR_xcb_surface = vkelIsDeviceExtensionSupported(physicalDevice, NULL, "VK_KHR_xcb_surface");
	VKEL_KHR_xlib_surface = vkelIsDeviceExtensionSupported(physicalDevice, NULL, "VK_KHR_xlib_surface");
	VKEL_NV_glsl_shader = vkelIsDeviceExtensionSupported(physicalDevice, NULL, "VK_NV_glsl_shader");

	// Instance and device layer names
	VKEL_LAYER_GOOGLE_unique_objects = vkelIsDeviceLayerSupported(physicalDevice, "VK_LAYER_GOOGLE_unique_objects");
	VKEL_LAYER_LUNARG_api_dump = vkelIsDeviceLayerSupported(physicalDevice, "VK_LAYER_LUNARG_api_dump");
	VKEL_LAYER_LUNARG_device_limits = vkelIsDeviceLayerSupported(physicalDevice, "VK_LAYER_LUNARG_device_limits");
	VKEL_LAYER_LUNARG_draw_state = vkelIsDeviceLayerSupported(physicalDevice, "VK_LAYER_LUNARG_draw_state");
	VKEL_LAYER_LUNARG_image = vkelIsDeviceLayerSupported(physicalDevice, "VK_LAYER_LUNARG_image");
	VKEL_LAYER_LUNARG_mem_tracker = vkelIsDeviceLayerSupported(physicalDevice, "VK_LAYER_LUNARG_mem_tracker");
	VKEL_LAYER_LUNARG_object_tracker = vkelIsDeviceLayerSupported(physicalDevice, "VK_LAYER_LUNARG_object_tracker");
	VKEL_LAYER_LUNARG_param_checker = vkelIsDeviceLayerSupported(physicalDevice, "VK_LAYER_LUNARG_param_checker");
	VKEL_LAYER_LUNARG_screenshot = vkelIsDeviceLayerSupported(physicalDevice, "VK_LAYER_LUNARG_screenshot");
	VKEL_LAYER_LUNARG_swapchain = vkelIsDeviceLayerSupported(physicalDevice, "VK_LAYER_LUNARG_swapchain");
	VKEL_LAYER_LUNARG_threading = vkelIsDeviceLayerSupported(physicalDevice, "VK_LAYER_LUNARG_threading");
	VKEL_LAYER_LUNARG_vktrace = vkelIsDeviceLayerSupported(physicalDevice, "VK_LAYER_LUNARG_vktrace");

	
	return VK_TRUE;
}
	
void vkelUninit(void)
{
	if (vkelVkLibHandle)
	{
		vkelPlatformCloseLibrary(vkelVkLibHandle);
		vkelVkLibHandle = NULL;
	}
}
	

#ifdef __cplusplus
}
#endif /* __cplusplus */