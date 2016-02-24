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
//
// Notice
//     Copyright (c) 2016 Vallentin Source <mail@vallentinsource.com>
//
// Developers & Contributors
//     Vallentin Source <mail@vallentinsource.com>
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

#pragma once
#ifndef _vkel_h_
#define _vkel_h_ 1
#define _VKEL_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

// #define VK_NO_PROTOTYPES
#include "vulkan.h"

// #ifndef __vulkan_h_
// #	error vulkan.h not included
// #endif

// #ifndef VK_NO_PROTOTYPES
// #	error #define VK_NO_PROTOTYPES before including vulkan.h
// #endif

VkBool32 VKEL_KHR_surface;
VkBool32 VKEL_KHR_swapchain;
VkBool32 VKEL_KHR_display;
VkBool32 VKEL_KHR_display_swapchain;
VkBool32 VKEL_KHR_xlib_surface;
VkBool32 VKEL_KHR_xcb_surface;
VkBool32 VKEL_KHR_wayland_surface;
VkBool32 VKEL_KHR_mir_surface;
VkBool32 VKEL_KHR_android_surface;
VkBool32 VKEL_KHR_win32_surface;
VkBool32 VKEL_EXT_debug_report;
PFN_vkVoidFunction vkelGetProcAddr(const char *name);
PFN_vkVoidFunction vkelGetInstanceProcAddr(VkInstance instance, const char *pName);

VkBool32 vkelIsSupported(const char *extension);

VkBool32 vkelInit(void);
void vkelUninit(void);

PFN_vkAllocationFunction __vkAllocationFunction;
#define vkAllocationFunction __vkAllocationFunction
PFN_vkReallocationFunction __vkReallocationFunction;
#define vkReallocationFunction __vkReallocationFunction
PFN_vkFreeFunction __vkFreeFunction;
#define vkFreeFunction __vkFreeFunction
PFN_vkInternalAllocationNotification __vkInternalAllocationNotification;
#define vkInternalAllocationNotification __vkInternalAllocationNotification
PFN_vkInternalFreeNotification __vkInternalFreeNotification;
#define vkInternalFreeNotification __vkInternalFreeNotification
PFN_vkVoidFunction __vkVoidFunction;
#define vkVoidFunction __vkVoidFunction
PFN_vkCreateInstance __vkCreateInstance;
#define vkCreateInstance __vkCreateInstance
PFN_vkDestroyInstance __vkDestroyInstance;
#define vkDestroyInstance __vkDestroyInstance
PFN_vkEnumeratePhysicalDevices __vkEnumeratePhysicalDevices;
#define vkEnumeratePhysicalDevices __vkEnumeratePhysicalDevices
PFN_vkGetPhysicalDeviceFeatures __vkGetPhysicalDeviceFeatures;
#define vkGetPhysicalDeviceFeatures __vkGetPhysicalDeviceFeatures
PFN_vkGetPhysicalDeviceFormatProperties __vkGetPhysicalDeviceFormatProperties;
#define vkGetPhysicalDeviceFormatProperties __vkGetPhysicalDeviceFormatProperties
PFN_vkGetPhysicalDeviceImageFormatProperties __vkGetPhysicalDeviceImageFormatProperties;
#define vkGetPhysicalDeviceImageFormatProperties __vkGetPhysicalDeviceImageFormatProperties
PFN_vkGetPhysicalDeviceProperties __vkGetPhysicalDeviceProperties;
#define vkGetPhysicalDeviceProperties __vkGetPhysicalDeviceProperties
PFN_vkGetPhysicalDeviceQueueFamilyProperties __vkGetPhysicalDeviceQueueFamilyProperties;
#define vkGetPhysicalDeviceQueueFamilyProperties __vkGetPhysicalDeviceQueueFamilyProperties
PFN_vkGetPhysicalDeviceMemoryProperties __vkGetPhysicalDeviceMemoryProperties;
#define vkGetPhysicalDeviceMemoryProperties __vkGetPhysicalDeviceMemoryProperties
PFN_vkGetInstanceProcAddr __vkGetInstanceProcAddr;
#define vkGetInstanceProcAddr __vkGetInstanceProcAddr
PFN_vkGetDeviceProcAddr __vkGetDeviceProcAddr;
#define vkGetDeviceProcAddr __vkGetDeviceProcAddr
PFN_vkCreateDevice __vkCreateDevice;
#define vkCreateDevice __vkCreateDevice
PFN_vkDestroyDevice __vkDestroyDevice;
#define vkDestroyDevice __vkDestroyDevice
PFN_vkEnumerateInstanceExtensionProperties __vkEnumerateInstanceExtensionProperties;
#define vkEnumerateInstanceExtensionProperties __vkEnumerateInstanceExtensionProperties
PFN_vkEnumerateDeviceExtensionProperties __vkEnumerateDeviceExtensionProperties;
#define vkEnumerateDeviceExtensionProperties __vkEnumerateDeviceExtensionProperties
PFN_vkEnumerateInstanceLayerProperties __vkEnumerateInstanceLayerProperties;
#define vkEnumerateInstanceLayerProperties __vkEnumerateInstanceLayerProperties
PFN_vkEnumerateDeviceLayerProperties __vkEnumerateDeviceLayerProperties;
#define vkEnumerateDeviceLayerProperties __vkEnumerateDeviceLayerProperties
PFN_vkGetDeviceQueue __vkGetDeviceQueue;
#define vkGetDeviceQueue __vkGetDeviceQueue
PFN_vkQueueSubmit __vkQueueSubmit;
#define vkQueueSubmit __vkQueueSubmit
PFN_vkQueueWaitIdle __vkQueueWaitIdle;
#define vkQueueWaitIdle __vkQueueWaitIdle
PFN_vkDeviceWaitIdle __vkDeviceWaitIdle;
#define vkDeviceWaitIdle __vkDeviceWaitIdle
PFN_vkAllocateMemory __vkAllocateMemory;
#define vkAllocateMemory __vkAllocateMemory
PFN_vkFreeMemory __vkFreeMemory;
#define vkFreeMemory __vkFreeMemory
PFN_vkMapMemory __vkMapMemory;
#define vkMapMemory __vkMapMemory
PFN_vkUnmapMemory __vkUnmapMemory;
#define vkUnmapMemory __vkUnmapMemory
PFN_vkFlushMappedMemoryRanges __vkFlushMappedMemoryRanges;
#define vkFlushMappedMemoryRanges __vkFlushMappedMemoryRanges
PFN_vkInvalidateMappedMemoryRanges __vkInvalidateMappedMemoryRanges;
#define vkInvalidateMappedMemoryRanges __vkInvalidateMappedMemoryRanges
PFN_vkGetDeviceMemoryCommitment __vkGetDeviceMemoryCommitment;
#define vkGetDeviceMemoryCommitment __vkGetDeviceMemoryCommitment
PFN_vkBindBufferMemory __vkBindBufferMemory;
#define vkBindBufferMemory __vkBindBufferMemory
PFN_vkBindImageMemory __vkBindImageMemory;
#define vkBindImageMemory __vkBindImageMemory
PFN_vkGetBufferMemoryRequirements __vkGetBufferMemoryRequirements;
#define vkGetBufferMemoryRequirements __vkGetBufferMemoryRequirements
PFN_vkGetImageMemoryRequirements __vkGetImageMemoryRequirements;
#define vkGetImageMemoryRequirements __vkGetImageMemoryRequirements
PFN_vkGetImageSparseMemoryRequirements __vkGetImageSparseMemoryRequirements;
#define vkGetImageSparseMemoryRequirements __vkGetImageSparseMemoryRequirements
PFN_vkGetPhysicalDeviceSparseImageFormatProperties __vkGetPhysicalDeviceSparseImageFormatProperties;
#define vkGetPhysicalDeviceSparseImageFormatProperties __vkGetPhysicalDeviceSparseImageFormatProperties
PFN_vkQueueBindSparse __vkQueueBindSparse;
#define vkQueueBindSparse __vkQueueBindSparse
PFN_vkCreateFence __vkCreateFence;
#define vkCreateFence __vkCreateFence
PFN_vkDestroyFence __vkDestroyFence;
#define vkDestroyFence __vkDestroyFence
PFN_vkResetFences __vkResetFences;
#define vkResetFences __vkResetFences
PFN_vkGetFenceStatus __vkGetFenceStatus;
#define vkGetFenceStatus __vkGetFenceStatus
PFN_vkWaitForFences __vkWaitForFences;
#define vkWaitForFences __vkWaitForFences
PFN_vkCreateSemaphore __vkCreateSemaphore;
#define vkCreateSemaphore __vkCreateSemaphore
PFN_vkDestroySemaphore __vkDestroySemaphore;
#define vkDestroySemaphore __vkDestroySemaphore
PFN_vkCreateEvent __vkCreateEvent;
#define vkCreateEvent __vkCreateEvent
PFN_vkDestroyEvent __vkDestroyEvent;
#define vkDestroyEvent __vkDestroyEvent
PFN_vkGetEventStatus __vkGetEventStatus;
#define vkGetEventStatus __vkGetEventStatus
PFN_vkSetEvent __vkSetEvent;
#define vkSetEvent __vkSetEvent
PFN_vkResetEvent __vkResetEvent;
#define vkResetEvent __vkResetEvent
PFN_vkCreateQueryPool __vkCreateQueryPool;
#define vkCreateQueryPool __vkCreateQueryPool
PFN_vkDestroyQueryPool __vkDestroyQueryPool;
#define vkDestroyQueryPool __vkDestroyQueryPool
PFN_vkGetQueryPoolResults __vkGetQueryPoolResults;
#define vkGetQueryPoolResults __vkGetQueryPoolResults
PFN_vkCreateBuffer __vkCreateBuffer;
#define vkCreateBuffer __vkCreateBuffer
PFN_vkDestroyBuffer __vkDestroyBuffer;
#define vkDestroyBuffer __vkDestroyBuffer
PFN_vkCreateBufferView __vkCreateBufferView;
#define vkCreateBufferView __vkCreateBufferView
PFN_vkDestroyBufferView __vkDestroyBufferView;
#define vkDestroyBufferView __vkDestroyBufferView
PFN_vkCreateImage __vkCreateImage;
#define vkCreateImage __vkCreateImage
PFN_vkDestroyImage __vkDestroyImage;
#define vkDestroyImage __vkDestroyImage
PFN_vkGetImageSubresourceLayout __vkGetImageSubresourceLayout;
#define vkGetImageSubresourceLayout __vkGetImageSubresourceLayout
PFN_vkCreateImageView __vkCreateImageView;
#define vkCreateImageView __vkCreateImageView
PFN_vkDestroyImageView __vkDestroyImageView;
#define vkDestroyImageView __vkDestroyImageView
PFN_vkCreateShaderModule __vkCreateShaderModule;
#define vkCreateShaderModule __vkCreateShaderModule
PFN_vkDestroyShaderModule __vkDestroyShaderModule;
#define vkDestroyShaderModule __vkDestroyShaderModule
PFN_vkCreatePipelineCache __vkCreatePipelineCache;
#define vkCreatePipelineCache __vkCreatePipelineCache
PFN_vkDestroyPipelineCache __vkDestroyPipelineCache;
#define vkDestroyPipelineCache __vkDestroyPipelineCache
PFN_vkGetPipelineCacheData __vkGetPipelineCacheData;
#define vkGetPipelineCacheData __vkGetPipelineCacheData
PFN_vkMergePipelineCaches __vkMergePipelineCaches;
#define vkMergePipelineCaches __vkMergePipelineCaches
PFN_vkCreateGraphicsPipelines __vkCreateGraphicsPipelines;
#define vkCreateGraphicsPipelines __vkCreateGraphicsPipelines
PFN_vkCreateComputePipelines __vkCreateComputePipelines;
#define vkCreateComputePipelines __vkCreateComputePipelines
PFN_vkDestroyPipeline __vkDestroyPipeline;
#define vkDestroyPipeline __vkDestroyPipeline
PFN_vkCreatePipelineLayout __vkCreatePipelineLayout;
#define vkCreatePipelineLayout __vkCreatePipelineLayout
PFN_vkDestroyPipelineLayout __vkDestroyPipelineLayout;
#define vkDestroyPipelineLayout __vkDestroyPipelineLayout
PFN_vkCreateSampler __vkCreateSampler;
#define vkCreateSampler __vkCreateSampler
PFN_vkDestroySampler __vkDestroySampler;
#define vkDestroySampler __vkDestroySampler
PFN_vkCreateDescriptorSetLayout __vkCreateDescriptorSetLayout;
#define vkCreateDescriptorSetLayout __vkCreateDescriptorSetLayout
PFN_vkDestroyDescriptorSetLayout __vkDestroyDescriptorSetLayout;
#define vkDestroyDescriptorSetLayout __vkDestroyDescriptorSetLayout
PFN_vkCreateDescriptorPool __vkCreateDescriptorPool;
#define vkCreateDescriptorPool __vkCreateDescriptorPool
PFN_vkDestroyDescriptorPool __vkDestroyDescriptorPool;
#define vkDestroyDescriptorPool __vkDestroyDescriptorPool
PFN_vkResetDescriptorPool __vkResetDescriptorPool;
#define vkResetDescriptorPool __vkResetDescriptorPool
PFN_vkAllocateDescriptorSets __vkAllocateDescriptorSets;
#define vkAllocateDescriptorSets __vkAllocateDescriptorSets
PFN_vkFreeDescriptorSets __vkFreeDescriptorSets;
#define vkFreeDescriptorSets __vkFreeDescriptorSets
PFN_vkUpdateDescriptorSets __vkUpdateDescriptorSets;
#define vkUpdateDescriptorSets __vkUpdateDescriptorSets
PFN_vkCreateFramebuffer __vkCreateFramebuffer;
#define vkCreateFramebuffer __vkCreateFramebuffer
PFN_vkDestroyFramebuffer __vkDestroyFramebuffer;
#define vkDestroyFramebuffer __vkDestroyFramebuffer
PFN_vkCreateRenderPass __vkCreateRenderPass;
#define vkCreateRenderPass __vkCreateRenderPass
PFN_vkDestroyRenderPass __vkDestroyRenderPass;
#define vkDestroyRenderPass __vkDestroyRenderPass
PFN_vkGetRenderAreaGranularity __vkGetRenderAreaGranularity;
#define vkGetRenderAreaGranularity __vkGetRenderAreaGranularity
PFN_vkCreateCommandPool __vkCreateCommandPool;
#define vkCreateCommandPool __vkCreateCommandPool
PFN_vkDestroyCommandPool __vkDestroyCommandPool;
#define vkDestroyCommandPool __vkDestroyCommandPool
PFN_vkResetCommandPool __vkResetCommandPool;
#define vkResetCommandPool __vkResetCommandPool
PFN_vkAllocateCommandBuffers __vkAllocateCommandBuffers;
#define vkAllocateCommandBuffers __vkAllocateCommandBuffers
PFN_vkFreeCommandBuffers __vkFreeCommandBuffers;
#define vkFreeCommandBuffers __vkFreeCommandBuffers
PFN_vkBeginCommandBuffer __vkBeginCommandBuffer;
#define vkBeginCommandBuffer __vkBeginCommandBuffer
PFN_vkEndCommandBuffer __vkEndCommandBuffer;
#define vkEndCommandBuffer __vkEndCommandBuffer
PFN_vkResetCommandBuffer __vkResetCommandBuffer;
#define vkResetCommandBuffer __vkResetCommandBuffer
PFN_vkCmdBindPipeline __vkCmdBindPipeline;
#define vkCmdBindPipeline __vkCmdBindPipeline
PFN_vkCmdSetViewport __vkCmdSetViewport;
#define vkCmdSetViewport __vkCmdSetViewport
PFN_vkCmdSetScissor __vkCmdSetScissor;
#define vkCmdSetScissor __vkCmdSetScissor
PFN_vkCmdSetLineWidth __vkCmdSetLineWidth;
#define vkCmdSetLineWidth __vkCmdSetLineWidth
PFN_vkCmdSetDepthBias __vkCmdSetDepthBias;
#define vkCmdSetDepthBias __vkCmdSetDepthBias
PFN_vkCmdSetBlendConstants __vkCmdSetBlendConstants;
#define vkCmdSetBlendConstants __vkCmdSetBlendConstants
PFN_vkCmdSetDepthBounds __vkCmdSetDepthBounds;
#define vkCmdSetDepthBounds __vkCmdSetDepthBounds
PFN_vkCmdSetStencilCompareMask __vkCmdSetStencilCompareMask;
#define vkCmdSetStencilCompareMask __vkCmdSetStencilCompareMask
PFN_vkCmdSetStencilWriteMask __vkCmdSetStencilWriteMask;
#define vkCmdSetStencilWriteMask __vkCmdSetStencilWriteMask
PFN_vkCmdSetStencilReference __vkCmdSetStencilReference;
#define vkCmdSetStencilReference __vkCmdSetStencilReference
PFN_vkCmdBindDescriptorSets __vkCmdBindDescriptorSets;
#define vkCmdBindDescriptorSets __vkCmdBindDescriptorSets
PFN_vkCmdBindIndexBuffer __vkCmdBindIndexBuffer;
#define vkCmdBindIndexBuffer __vkCmdBindIndexBuffer
PFN_vkCmdBindVertexBuffers __vkCmdBindVertexBuffers;
#define vkCmdBindVertexBuffers __vkCmdBindVertexBuffers
PFN_vkCmdDraw __vkCmdDraw;
#define vkCmdDraw __vkCmdDraw
PFN_vkCmdDrawIndexed __vkCmdDrawIndexed;
#define vkCmdDrawIndexed __vkCmdDrawIndexed
PFN_vkCmdDrawIndirect __vkCmdDrawIndirect;
#define vkCmdDrawIndirect __vkCmdDrawIndirect
PFN_vkCmdDrawIndexedIndirect __vkCmdDrawIndexedIndirect;
#define vkCmdDrawIndexedIndirect __vkCmdDrawIndexedIndirect
PFN_vkCmdDispatch __vkCmdDispatch;
#define vkCmdDispatch __vkCmdDispatch
PFN_vkCmdDispatchIndirect __vkCmdDispatchIndirect;
#define vkCmdDispatchIndirect __vkCmdDispatchIndirect
PFN_vkCmdCopyBuffer __vkCmdCopyBuffer;
#define vkCmdCopyBuffer __vkCmdCopyBuffer
PFN_vkCmdCopyImage __vkCmdCopyImage;
#define vkCmdCopyImage __vkCmdCopyImage
PFN_vkCmdBlitImage __vkCmdBlitImage;
#define vkCmdBlitImage __vkCmdBlitImage
PFN_vkCmdCopyBufferToImage __vkCmdCopyBufferToImage;
#define vkCmdCopyBufferToImage __vkCmdCopyBufferToImage
PFN_vkCmdCopyImageToBuffer __vkCmdCopyImageToBuffer;
#define vkCmdCopyImageToBuffer __vkCmdCopyImageToBuffer
PFN_vkCmdUpdateBuffer __vkCmdUpdateBuffer;
#define vkCmdUpdateBuffer __vkCmdUpdateBuffer
PFN_vkCmdFillBuffer __vkCmdFillBuffer;
#define vkCmdFillBuffer __vkCmdFillBuffer
PFN_vkCmdClearColorImage __vkCmdClearColorImage;
#define vkCmdClearColorImage __vkCmdClearColorImage
PFN_vkCmdClearDepthStencilImage __vkCmdClearDepthStencilImage;
#define vkCmdClearDepthStencilImage __vkCmdClearDepthStencilImage
PFN_vkCmdClearAttachments __vkCmdClearAttachments;
#define vkCmdClearAttachments __vkCmdClearAttachments
PFN_vkCmdResolveImage __vkCmdResolveImage;
#define vkCmdResolveImage __vkCmdResolveImage
PFN_vkCmdSetEvent __vkCmdSetEvent;
#define vkCmdSetEvent __vkCmdSetEvent
PFN_vkCmdResetEvent __vkCmdResetEvent;
#define vkCmdResetEvent __vkCmdResetEvent
PFN_vkCmdWaitEvents __vkCmdWaitEvents;
#define vkCmdWaitEvents __vkCmdWaitEvents
PFN_vkCmdPipelineBarrier __vkCmdPipelineBarrier;
#define vkCmdPipelineBarrier __vkCmdPipelineBarrier
PFN_vkCmdBeginQuery __vkCmdBeginQuery;
#define vkCmdBeginQuery __vkCmdBeginQuery
PFN_vkCmdEndQuery __vkCmdEndQuery;
#define vkCmdEndQuery __vkCmdEndQuery
PFN_vkCmdResetQueryPool __vkCmdResetQueryPool;
#define vkCmdResetQueryPool __vkCmdResetQueryPool
PFN_vkCmdWriteTimestamp __vkCmdWriteTimestamp;
#define vkCmdWriteTimestamp __vkCmdWriteTimestamp
PFN_vkCmdCopyQueryPoolResults __vkCmdCopyQueryPoolResults;
#define vkCmdCopyQueryPoolResults __vkCmdCopyQueryPoolResults
PFN_vkCmdPushConstants __vkCmdPushConstants;
#define vkCmdPushConstants __vkCmdPushConstants
PFN_vkCmdBeginRenderPass __vkCmdBeginRenderPass;
#define vkCmdBeginRenderPass __vkCmdBeginRenderPass
PFN_vkCmdNextSubpass __vkCmdNextSubpass;
#define vkCmdNextSubpass __vkCmdNextSubpass
PFN_vkCmdEndRenderPass __vkCmdEndRenderPass;
#define vkCmdEndRenderPass __vkCmdEndRenderPass
PFN_vkCmdExecuteCommands __vkCmdExecuteCommands;
#define vkCmdExecuteCommands __vkCmdExecuteCommands
PFN_vkDestroySurfaceKHR __vkDestroySurfaceKHR;
#define vkDestroySurfaceKHR __vkDestroySurfaceKHR
PFN_vkGetPhysicalDeviceSurfaceSupportKHR __vkGetPhysicalDeviceSurfaceSupportKHR;
#define vkGetPhysicalDeviceSurfaceSupportKHR __vkGetPhysicalDeviceSurfaceSupportKHR
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR __vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
#define vkGetPhysicalDeviceSurfaceCapabilitiesKHR __vkGetPhysicalDeviceSurfaceCapabilitiesKHR
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR __vkGetPhysicalDeviceSurfaceFormatsKHR;
#define vkGetPhysicalDeviceSurfaceFormatsKHR __vkGetPhysicalDeviceSurfaceFormatsKHR
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR __vkGetPhysicalDeviceSurfacePresentModesKHR;
#define vkGetPhysicalDeviceSurfacePresentModesKHR __vkGetPhysicalDeviceSurfacePresentModesKHR
PFN_vkCreateSwapchainKHR __vkCreateSwapchainKHR;
#define vkCreateSwapchainKHR __vkCreateSwapchainKHR
PFN_vkDestroySwapchainKHR __vkDestroySwapchainKHR;
#define vkDestroySwapchainKHR __vkDestroySwapchainKHR
PFN_vkGetSwapchainImagesKHR __vkGetSwapchainImagesKHR;
#define vkGetSwapchainImagesKHR __vkGetSwapchainImagesKHR
PFN_vkAcquireNextImageKHR __vkAcquireNextImageKHR;
#define vkAcquireNextImageKHR __vkAcquireNextImageKHR
PFN_vkQueuePresentKHR __vkQueuePresentKHR;
#define vkQueuePresentKHR __vkQueuePresentKHR
PFN_vkGetPhysicalDeviceDisplayPropertiesKHR __vkGetPhysicalDeviceDisplayPropertiesKHR;
#define vkGetPhysicalDeviceDisplayPropertiesKHR __vkGetPhysicalDeviceDisplayPropertiesKHR
PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR __vkGetPhysicalDeviceDisplayPlanePropertiesKHR;
#define vkGetPhysicalDeviceDisplayPlanePropertiesKHR __vkGetPhysicalDeviceDisplayPlanePropertiesKHR
PFN_vkGetDisplayPlaneSupportedDisplaysKHR __vkGetDisplayPlaneSupportedDisplaysKHR;
#define vkGetDisplayPlaneSupportedDisplaysKHR __vkGetDisplayPlaneSupportedDisplaysKHR
PFN_vkGetDisplayModePropertiesKHR __vkGetDisplayModePropertiesKHR;
#define vkGetDisplayModePropertiesKHR __vkGetDisplayModePropertiesKHR
PFN_vkCreateDisplayModeKHR __vkCreateDisplayModeKHR;
#define vkCreateDisplayModeKHR __vkCreateDisplayModeKHR
PFN_vkGetDisplayPlaneCapabilitiesKHR __vkGetDisplayPlaneCapabilitiesKHR;
#define vkGetDisplayPlaneCapabilitiesKHR __vkGetDisplayPlaneCapabilitiesKHR
PFN_vkCreateDisplayPlaneSurfaceKHR __vkCreateDisplayPlaneSurfaceKHR;
#define vkCreateDisplayPlaneSurfaceKHR __vkCreateDisplayPlaneSurfaceKHR
PFN_vkCreateSharedSwapchainsKHR __vkCreateSharedSwapchainsKHR;
#define vkCreateSharedSwapchainsKHR __vkCreateSharedSwapchainsKHR
PFN_vkDebugReportCallbackEXT __vkDebugReportCallbackEXT;
#define vkDebugReportCallbackEXT __vkDebugReportCallbackEXT
PFN_vkCreateDebugReportCallbackEXT __vkCreateDebugReportCallbackEXT;
#define vkCreateDebugReportCallbackEXT __vkCreateDebugReportCallbackEXT
PFN_vkDestroyDebugReportCallbackEXT __vkDestroyDebugReportCallbackEXT;
#define vkDestroyDebugReportCallbackEXT __vkDestroyDebugReportCallbackEXT
PFN_vkDebugReportMessageEXT __vkDebugReportMessageEXT;
#define vkDebugReportMessageEXT __vkDebugReportMessageEXT

#define vkCreateWin32SurfaceKHR NULL
#define vkGetPhysicalDeviceWin32PresentationSupportKHR NULL

#ifdef VK_USE_PLATFORM_WIN32_KHR
PFN_vkCreateWin32SurfaceKHR __vkCreateWin32SurfaceKHR;
#undef vkCreateWin32SurfaceKHR
#define vkCreateWin32SurfaceKHR __vkCreateWin32SurfaceKHR
PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR __vkGetPhysicalDeviceWin32PresentationSupportKHR;
#undef vkGetPhysicalDeviceWin32PresentationSupportKHR
#define vkGetPhysicalDeviceWin32PresentationSupportKHR __vkGetPhysicalDeviceWin32PresentationSupportKHR
#endif /* VK_USE_PLATFORM_WIN32_KHR */

#define vkCreateWaylandSurfaceKHR NULL
#define vkGetPhysicalDeviceWaylandPresentationSupportKHR NULL

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
PFN_vkCreateWaylandSurfaceKHR __vkCreateWaylandSurfaceKHR;
#undef vkCreateWaylandSurfaceKHR
#define vkCreateWaylandSurfaceKHR __vkCreateWaylandSurfaceKHR
PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR __vkGetPhysicalDeviceWaylandPresentationSupportKHR;
#undef vkGetPhysicalDeviceWaylandPresentationSupportKHR
#define vkGetPhysicalDeviceWaylandPresentationSupportKHR __vkGetPhysicalDeviceWaylandPresentationSupportKHR
#endif /* VK_USE_PLATFORM_WAYLAND_KHR */

#define vkCreateXlibSurfaceKHR NULL
#define vkGetPhysicalDeviceXlibPresentationSupportKHR NULL

#ifdef VK_USE_PLATFORM_XLIB_KHR
PFN_vkCreateXlibSurfaceKHR __vkCreateXlibSurfaceKHR;
#undef vkCreateXlibSurfaceKHR
#define vkCreateXlibSurfaceKHR __vkCreateXlibSurfaceKHR
PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR __vkGetPhysicalDeviceXlibPresentationSupportKHR;
#undef vkGetPhysicalDeviceXlibPresentationSupportKHR
#define vkGetPhysicalDeviceXlibPresentationSupportKHR __vkGetPhysicalDeviceXlibPresentationSupportKHR
#endif /* VK_USE_PLATFORM_XLIB_KHR */

#define vkCreateAndroidSurfaceKHR NULL

#ifdef VK_USE_PLATFORM_ANDROID_KHR
PFN_vkCreateAndroidSurfaceKHR __vkCreateAndroidSurfaceKHR;
#undef vkCreateAndroidSurfaceKHR
#define vkCreateAndroidSurfaceKHR __vkCreateAndroidSurfaceKHR
#endif /* VK_USE_PLATFORM_ANDROID_KHR */

#define vkCreateMirSurfaceKHR NULL
#define vkGetPhysicalDeviceMirPresentationSupportKHR NULL

#ifdef VK_USE_PLATFORM_MIR_KHR
PFN_vkCreateMirSurfaceKHR __vkCreateMirSurfaceKHR;
#undef vkCreateMirSurfaceKHR
#define vkCreateMirSurfaceKHR __vkCreateMirSurfaceKHR
PFN_vkGetPhysicalDeviceMirPresentationSupportKHR __vkGetPhysicalDeviceMirPresentationSupportKHR;
#undef vkGetPhysicalDeviceMirPresentationSupportKHR
#define vkGetPhysicalDeviceMirPresentationSupportKHR __vkGetPhysicalDeviceMirPresentationSupportKHR
#endif /* VK_USE_PLATFORM_MIR_KHR */

#define vkCreateXcbSurfaceKHR NULL
#define vkGetPhysicalDeviceXcbPresentationSupportKHR NULL

#ifdef VK_USE_PLATFORM_XCB_KHR
PFN_vkCreateXcbSurfaceKHR __vkCreateXcbSurfaceKHR;
#undef vkCreateXcbSurfaceKHR
#define vkCreateXcbSurfaceKHR __vkCreateXcbSurfaceKHR
PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR __vkGetPhysicalDeviceXcbPresentationSupportKHR;
#undef vkGetPhysicalDeviceXcbPresentationSupportKHR
#define vkGetPhysicalDeviceXcbPresentationSupportKHR __vkGetPhysicalDeviceXcbPresentationSupportKHR
#endif /* VK_USE_PLATFORM_XCB_KHR */

#ifdef __cplusplus
}
#endif

#endif /* _vkel_h_ */