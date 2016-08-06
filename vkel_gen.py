#!/usr/bin/env python

# Allow Python 2.6+ to use the print() function
from __future__ import print_function

COPYRIGHT = br'''//========================================================================
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
//     Last Modified Date: May 16, 2016
//     Revision: 14
//     Version: 2.0.11
//
// Revision History
//     Revision 12, 2016/05/02
//       - Fixed "for loop initial declarations are only
//         allowed in C99 mode".
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

'''


# Try to import Python 3 library urllib.request
# and if it fails, fall back to Python 2 urllib2
try:
	import urllib.request as urllib2
except ImportError:
	import urllib2

import os
import re


import sys

# The first argument is always the script name,
# we don't really care about that, so remove it!
argv = sys.argv[1:]
argc = len(argv)

force = False
keep = False

if '-f' in argv:
	force = True

if '-k' in argv:
	keep = True


vulkan_h = ""
vk_platform_h = ""


if not force and os.path.exists("vulkan.h"):
	print("Using existing vulkan.h")
	
	with open("vulkan.h", "r") as f:
		for line in f:
			vulkan_h += line
else:
	print("Downloading vulkan.h from GitHub")
	print("URL: https://github.com/KhronosGroup/Vulkan-Docs/tree/1.0/src/vulkan")
	
	vulkan_h_url = "http://raw.githubusercontent.com/KhronosGroup/Vulkan-Docs/1.0/src/vulkan/vulkan.h"
	vulkan_h_web = urllib2.urlopen(vulkan_h_url)
	
	for line in vulkan_h_web.readlines():
		vulkan_h += line.decode("utf-8")
	
	if keep:
		with open("vulkan.h", "wb") as f:
			#f.writelines(vulkan_h_web.readlines())
			f.write(vulkan_h.encode("utf-8"))


if not force and os.path.exists("vk_platform.h"):
	print("Using existing vk_platform.h")
	
	with open("vk_platform.h", "r") as f:
		for line in f:
			vk_platform_h += line
else:
	print("Downloading vk_platform.h from GitHub")
	print("URL: https://github.com/KhronosGroup/Vulkan-Docs/tree/1.0/src/vulkan")
	
	vk_platform_h_url = "https://raw.githubusercontent.com/KhronosGroup/Vulkan-Docs/1.0/src/vulkan/vk_platform.h"
	vk_platform_h_web = urllib2.urlopen(vk_platform_h_url)
	
	for line in vk_platform_h_web.readlines():
		vk_platform_h += line.decode("utf-8")
	
	if keep:
		with open("vk_platform.h", "wb") as f:
			#f.writelines(vk_platform_h_web.readlines())
			f.write(vk_platform_h.encode("utf-8"))


print("Parsing vulkan.h and vk_platform.h")


extension_names = []

for extension_name in re.findall("#define\s+.*?_EXTENSION_NAME\s+\"VK_(.*?)\"", vulkan_h):
	extension_names.append(extension_name)


# This device extension isn't listed in vulkan.h, to just manually add it for now
if "NV_glsl_shader" not in extension_names:
	extension_names.append("NV_glsl_shader")


# Layers aren't defined in vulkan.h (at the point of writing this at least),
# so I need to find an easier way of automating. Currently they are hand
# written using the output of vkEnumerateInstanceLayerProperties.
layer_names = [
	# Instance Layers
	"LAYER_LUNARG_api_dump",
	"LAYER_LUNARG_device_limits",
	"LAYER_LUNARG_draw_state",
	"LAYER_LUNARG_image",
	"LAYER_LUNARG_mem_tracker",
	"LAYER_LUNARG_object_tracker",
	"LAYER_LUNARG_param_checker",
	"LAYER_LUNARG_screenshot",
	"LAYER_LUNARG_swapchain",
	"LAYER_LUNARG_threading",
	"LAYER_GOOGLE_unique_objects",
	"LAYER_LUNARG_vktrace",
	
	# Device Layers
	"",
]


# all the functions no matter which platform
all_funcs = []

# all the function but platform specific
platform_funcs = {
	"": [] # none-platform specific functions
}

# current platform
platform = None


# regex_function = re.compile(r"PFN_(\w+)")
regex_function = re.compile(r"VKAPI_PTR\s+\*PFN_(\w+)")

regex_platform_begin = re.compile(r"#ifdef\s+(VK_USE_PLATFORM\w+)")
regex_platform_end = re.compile(r"#endif\s+/\*\s*(\w+)\s*\*/")


for line in vulkan_h.splitlines():
	match_platform = regex_platform_begin.search(line)
	
	if match_platform:
			platform = match_platform.group(1)
			
			if platform not in platform_funcs:
				platform_funcs[platform] = []
	
	
	match_platform = regex_platform_end.search(line)
	
	if match_platform:
		if platform == match_platform.group(1):
			platform = None
	
	
	match_func = regex_function.search(line)
	
	if match_func:
		func = match_func.group(1)
		
		if func not in all_funcs:
			all_funcs.append(func)
			
			if platform is None:
				platform_funcs[""].append(func)
			else:
				platform_funcs[platform].append(func)


print("Processing vulkan.h and vk_platform.h")


def process_source_code(source):
	# Remove extern (we gonna add one overall later)
	source = re.compile("\s*?#\s*?ifdef\s+__cplusplus.*?#endif.*?$", re.S | re.I | re.M).sub("", source)
	
	# Remove everything within VK_NO_PROTOTYPES
	source = re.compile(r"^\s*?#\s*?ifndef VK_NO_PROTOTYPES.*?^#\s*endif", re.S | re.M).sub("", source)
	
	
	# Remove multi-line comments
	source = re.compile("\/\*[\s\S]*?\*\/").sub("", source)
	
	# Remove single-line comments, which are at the beginning of a line
	source = re.compile("^//.*?\r?\n", re.M).sub("", source)
	
	
	# Trim lines that only contain whitespace
	source = re.compile("^\s+$", re.M).sub("", source)
	
	# Turn repeating new lines into 1 new line
	source = re.compile(r"^(\r?\n)\s*?(\r?\n\s*?)+^", re.M).sub(r"\1", source)
	
	# Trim leading and trailing whitespace
	source = source.strip()
	
	
	return source


# This works by selecting the longest found multi-line comment,
# that contains the word "copyright" case-insensitive.
def get_copyright(source):
	longest = ""
	
	re_copyright = re.compile("copyright", re.I)
	
	for comment in re.findall("\/\*[\s\S]*?\*\/", vulkan_h, re.M):
		if re_copyright.search(comment):
			if len(comment) > len(longest):
				longest = comment
	
	# Is it empty?
	if not longest:
		return None
	
	# Trim it
	longest = longest.strip()
	
	# Then add 2 new lines (just for formatting purposes in vkel)
	longest += "\n\n"
	
	return longest


vulkan_h_copyright = get_copyright(vulkan_h)
vk_platform_h_copyright = get_copyright(vulkan_h)

# Are they identical copyrights?
if vulkan_h_copyright == vk_platform_h_copyright:
	# Then just remove one of them
	vk_platform_h_copyright = None


vk_platform_h = process_source_code(vk_platform_h)
vulkan_h = process_source_code(vulkan_h)

vulkan_h = re.compile(r"^\s*#\s*include\s+\"vk_platform.h\"", re.I | re.M).sub(vk_platform_h, vulkan_h)

vulkan_h = process_source_code(vulkan_h)


print("Generating vkel.h")


def add_copyright(f):
	f.write(COPYRIGHT)
	
	if vulkan_h_copyright:
		f.write(vulkan_h_copyright.encode("utf-8"))
	
	if vk_platform_h_copyright:
		f.write(vk_platform_h_copyright.encode("utf-8"))


with open("vkel.h", "wb") as f:
	add_copyright(f)
	
	
	f.write(br'''#ifndef _VKEL_H_
#define _VKEL_H_ 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#if !defined(_WIN32) && (defined(__WIN32__) || defined(WIN32) || defined(__MINGW32__))
#	define _WIN32
#endif /* _WIN32 */

#if defined(_WIN32)
#	ifndef VK_USE_PLATFORM_WIN32_KHR
#		define VK_USE_PLATFORM_WIN32_KHR 1
#	endif
#elif defined(__ANDROID__) && defined(__ARM_EABI__) && !defined(__ARM_ARCH_7A__)
#	ifndef VK_USE_PLATFORM_ANDROID_KHR
#		define VK_USE_PLATFORM_ANDROID_KHR 1
#	endif
// #elif defined(__linux__) || defined(__unix__)
#else
#	ifndef VK_USE_PLATFORM_XCB_KHR
#		define VK_USE_PLATFORM_XCB_KHR 1
#	endif
// #else
// #	error You need to open vkel.h and add a #define VK_USE_PLATFORM_*_KHR. Please also report your OS on https://github.com/MrVallentin/vkel
#endif

''')
	
	
	f.write(br'''
#ifdef __vulkan_h_

#if _MSC_VER
#	pragma message("Warning: Vulkan 1.0.4's include guard was changed from __vulkan_h_ to VULKAN_H_, so you might need to update your vulkan.h file")
#elif __GNUC__
#	warning "Warning: Vulkan 1.0.4's include guard was changed from __vulkan_h_ to VULKAN_H_, so you might need to update your vulkan.h file"
#endif

#endif /* __vulkan_h_ */


''')
	
	# vulkan_h contains vk_platform_h
	f.write(vulkan_h.encode("utf-8"))
	f.write(b"\n\n")
	
	
	f.write(br'''
extern PFN_vkVoidFunction vkelGetProcAddr(const char *pName);

extern PFN_vkVoidFunction vkelGetInstanceProcAddr(VkInstance instance, const char *pName);
extern PFN_vkVoidFunction vkelGetDeviceProcAddr(VkDevice device, const char *pName);


extern void vkelDeleteNames(uint32_t nameCount, char **names);


extern char** vkelGetInstanceExtensionNames(const char *pLayerName, uint32_t *extensionNameCount);
#define vkelDeleteInstanceExtensionNames vkelDeleteNames

extern char** vkelGetInstanceLayerNames(uint32_t *layerNameCount);
#define vkelDeleteInstanceLayerNames vkelDeleteNames


extern char** vkelGetDeviceExtensionNames(VkPhysicalDevice physicalDevice, const char *pLayerName, uint32_t *extensionNameCount);
#define vkelDeleteDeviceExtensionNames vkelDeleteNames

extern char** vkelGetDeviceLayerNames(VkPhysicalDevice physicalDevice, uint32_t *layerNameCount);
#define vkelDeleteDeviceLayerNames vkelDeleteNames


extern VkBool32 vkelIsInstanceLayerSupported(const char *pLayerName);
extern VkBool32 vkelIsInstanceExtensionSupported(const char *pLayerName, const char *pExtensionName);

extern VkBool32 vkelIsDeviceLayerSupported(VkPhysicalDevice physicalDevice, const char *pLayerName);
extern VkBool32 vkelIsDeviceExtensionSupported(VkPhysicalDevice physicalDevice, const char *pLayerName, const char *pExtensionName);


extern VkBool32 vkelInit(void);
extern VkBool32 vkelInstanceInit(VkInstance instance);
extern VkBool32 vkelDeviceInit(VkPhysicalDevice physicalDevice, VkDevice device);

extern void vkelUninit(void);


''')
	
	
	f.write(b"// Instance and device extension names\n")
	
	# Are there any (instance or device) extensions? (is array empty)
	if extension_names:
		for extension_name in sorted(extension_names):
			if extension_name:
				f.write("extern VkBool32 VKEL_{0};\n".format(extension_name).encode("utf-8"))
	
	f.write(b"\n")
	
	
	f.write(b"// Instance and device layer names\n")
	
	# Are there any (instance or device) layers? (is array empty)
	if layer_names:
		for layer_name in sorted(layer_names):
			if layer_name:
				f.write("extern VkBool32 VKEL_{0};\n".format(layer_name).encode("utf-8"))
	
	f.write(b"\n\n")
	
	
	lines = []
	
	lines.append("// Functions")
	
	# Generate all function pointers
	for platform in sorted(platform_funcs):
		if platform:
			lines.append("#ifdef " + platform)
		
		for func in sorted(platform_funcs[platform]):
			# lines.append("PFN_{0} {0};".format(func))
			# lines.append("PFN_{0} __{0};".format(func))
			lines.append("extern PFN_{0} __{0};".format(func))
		
		
		if platform:
			lines.append("#endif /* " + platform + " */")
		
		lines.append("")
	
	# Generate NULL pointers for platform specific functions
	for platform in sorted(platform_funcs):
		if platform:
			lines.append("// {0}".format(platform))
			
			for func in sorted(platform_funcs[platform]):
				lines.append("#define {0} NULL".format(func))
			
			lines.append("")
		else:
			for func in sorted(platform_funcs[platform]):
				lines.append("#define {0} __{0}".format(func))
	
	# Generate platform specific functions
	for platform in sorted(platform_funcs):
		if platform:
			lines.append("#ifdef " + platform)
			
			for func in sorted(platform_funcs[platform]):
				lines.append("#undef {0}".format(func))
			
			for func in sorted(platform_funcs[platform]):
				lines.append("#define {0} __{0}".format(func))
			
			lines.append("#endif /* " + platform + " */")
			lines.append("")
	
	lines.append("")
	
	f.write("\n".join(lines).encode("utf-8"))
	
	
	f.write(br'''
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _VKEL_H_ */''')


print("Generating vkel.c")


with open("vkel.c", "wb") as f:
	add_copyright(f)
	
	
	f.write(br'''#include "vkel.h"

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


''')
	
	
	f.write(b"// Instance and device extension names\n")
	
	# Are there any (instance or device) extensions? (is array empty)
	if extension_names:
		for extension_name in sorted(extension_names):
			if extension_name:
				f.write("VkBool32 VKEL_{0};\n".format(extension_name).encode("utf-8"))
	
	f.write(b"\n")
	
	
	f.write(b"// Instance and device layer names\n")
	
	# Are there any (instance or device) layers? (is array empty)
	if layer_names:
		for layer_name in sorted(layer_names):
			if layer_name:
				f.write("VkBool32 VKEL_{0};\n".format(layer_name).encode("utf-8"))
	
	f.write(b"\n\n")
	
	
	lines = []
	
	lines.append("// Functions")
	
	# Generate all function pointers
	for platform in sorted(platform_funcs):
		if platform:
			lines.append("#ifdef " + platform)
		
		for func in sorted(platform_funcs[platform]):
			# lines.append("PFN_{0} {0};".format(func))
			lines.append("PFN_{0} __{0};".format(func))
		
		
		if platform:
			lines.append("#endif /* " + platform + " */")
		
		lines.append("")
	
	lines.append("")
	
	f.write("\n".join(lines).encode("utf-8"))
	
	
	
	f.write(br'''
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
	
	uint32_t nameIndex = 0;
	for (nameIndex = 0; nameIndex < nameCount; nameIndex++)
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
	
	uint32_t extensionNameIndex = 0;
	for (extensionNameIndex = 0; extensionNameIndex < extPropertyCount; extensionNameIndex++)
	{
		extensionNames[extensionNameIndex] = (char*)calloc(VK_MAX_EXTENSION_NAME_SIZE, sizeof(char));
	}


	VkExtensionProperties *extProperties = (VkExtensionProperties*) calloc(extPropertyCount, sizeof(VkExtensionProperties));
	assert(extProperties);


	err = vkEnumerateInstanceExtensionProperties(pLayerName, &extPropertyCount, extProperties);
	assert(!err);
	
	uint32_t extPropertyIndex = 0;
	for (extPropertyIndex = 0; extPropertyIndex < extPropertyCount; extPropertyIndex++)
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
	
	uint32_t layerNameIndex = 0;
	for (layerNameIndex = 0; layerNameIndex < layerPropertyCount; layerNameIndex++)
	{
		layerNames[layerNameIndex] = (char*) calloc(VK_MAX_EXTENSION_NAME_SIZE, sizeof(char));
	}


	VkLayerProperties *layerProperties = (VkLayerProperties*) calloc(layerPropertyCount, sizeof(VkLayerProperties));
	assert(layerProperties);

	err = vkEnumerateInstanceLayerProperties(&layerPropertyCount, layerProperties);
	assert(!err);
	
	uint32_t layerPropertyIndex = 0;
	for (layerPropertyIndex = 0; layerPropertyIndex < layerPropertyCount; layerPropertyIndex++)
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
	
	uint32_t extensionNameIndex = 0;
	for (extensionNameIndex = 0; extensionNameIndex < extPropertyCount; extensionNameIndex++)
	{
		extensionNames[extensionNameIndex] = (char*) calloc(VK_MAX_EXTENSION_NAME_SIZE, sizeof(char));
	}


	VkExtensionProperties *extProperties = (VkExtensionProperties*) calloc(extPropertyCount, sizeof(VkExtensionProperties));
	assert(extProperties);


	err = vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, &extPropertyCount, extProperties);
	assert(!err);
	
	uint32_t extPropertyIndex = 0;
	for (extPropertyIndex = 0; extPropertyIndex < extPropertyCount; extPropertyIndex++)
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
	
	uint32_t layerNameIndex = 0;
	for (layerNameIndex = 0; layerNameIndex < layerPropertyCount; layerNameIndex++)
	{
		layerNames[layerNameIndex] = (char*) calloc(VK_MAX_EXTENSION_NAME_SIZE, sizeof(char));
	}


	VkLayerProperties *layerProperties = (VkLayerProperties*) calloc(layerPropertyCount, sizeof(VkLayerProperties));
	assert(layerProperties);

	err = vkEnumerateDeviceLayerProperties(physicalDevice, &layerPropertyCount, layerProperties);
	assert(!err);
	
	uint32_t layerPropertyIndex = 0;
	for (layerPropertyIndex = 0; layerPropertyIndex < layerPropertyCount; layerPropertyIndex++)
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
	
	uint32_t layerNameIndex = 0;
	for (layerNameIndex = 0; layerNameIndex < layerNameCount; layerNameIndex++)
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
	
	uint32_t extensionNameIndex = 0;
	for (extensionNameIndex = 0; extensionNameIndex < extensionNameCount; extensionNameIndex++)
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
	
	uint32_t layerNameIndex = 0;
	for (layerNameIndex = 0; layerNameIndex < layerNameCount; layerNameIndex++)
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
	
	uint32_t extensionNameIndex = 0;
	for (extensionNameIndex = 0; extensionNameIndex < extensionNameCount; extensionNameIndex++)
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

''')
	
	
	# vkelInit()
		
	f.write(br'''
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
	
	
''')
	
	
	lines = []
	
	for platform in sorted(platform_funcs):
		if platform:
			lines.append("#ifdef " + platform)
		
		for func in sorted(platform_funcs[platform]):
			lines.append("\t__{0} = (PFN_{0}) vkelGetProcAddr(\"{0}\");".format(func))
		
		if platform:
			lines.append("#endif /* " + platform + " */")
		
		lines.append("")
	
	lines.append("")
	
	
	lines.append("\t// Instance and device extension names")
	
	# Are there any (instance or device) extensions? (is array empty)
	if extension_names:
		for extension_name in sorted(extension_names):
			if extension_name:
				lines.append("\tVKEL_" + extension_name + " = vkelIsInstanceExtensionSupported(NULL, \"VK_" + extension_name + "\");")
	
	lines.append("")
	
	
	lines.append("\t// Instance and device layer names")
	
	# Are there any (instance or device) layers? (is array empty)
	if layer_names:
		for layer_name in sorted(layer_names):
			if layer_name:
				lines.append("\tVKEL_" + layer_name + " = vkelIsInstanceLayerSupported(\"VK_" + layer_name + "\");")
	
	lines.append("")
	
	
	f.write("\n".join(lines).encode("utf-8"))
	
	
	f.write(br'''
	
	return VK_TRUE;
}
	''')
	
	
	# vkelInstanceInit()
		
	f.write(br'''
VkBool32 vkelInstanceInit(VkInstance instance)
{
	if (!vkelVkLibHandle && !vkelInit())
		return VK_FALSE;
	
	
''')
	
	
	lines = []
	
	for platform in sorted(platform_funcs):
		if platform:
			lines.append("#ifdef " + platform)
		
		for func in sorted(platform_funcs[platform]):
			lines.append("\t__{0} = (PFN_{0}) vkelGetInstanceProcAddr(instance, \"{0}\");".format(func))
		
		if platform:
			lines.append("#endif /* " + platform + " */")
		
		lines.append("")
	
	lines.append("")
	
	
	lines.append("\t// Instance and device extension names")
	
	# Are there any (instance or device) extensions? (is array empty)
	if extension_names:
		for extension_name in sorted(extension_names):
			if extension_name:
				lines.append("\tVKEL_" + extension_name + " = vkelIsInstanceExtensionSupported(NULL, \"VK_" + extension_name + "\");")
	
	lines.append("")
	
	
	lines.append("\t// Instance and device layer names")
	
	# Are there any (instance or device) layers? (is array empty)
	if layer_names:
		for layer_name in sorted(layer_names):
			if layer_name:
				lines.append("\tVKEL_" + layer_name + " = vkelIsInstanceLayerSupported(\"VK_" + layer_name + "\");")
	
	lines.append("")
	
	
	f.write("\n".join(lines).encode("utf-8"))
	
	
	f.write(br'''
	
	return VK_TRUE;
}
	''')
	
	
	# vkelDeviceInit()
	
	f.write(br'''
VkBool32 vkelDeviceInit(VkPhysicalDevice physicalDevice, VkDevice device)
{
	if (!vkelVkLibHandle && !vkelInit())
		return VK_FALSE;
	
	
''')
	
	
	lines = []
	
	for platform in sorted(platform_funcs):
		if platform:
			lines.append("#ifdef " + platform)
		
		for func in sorted(platform_funcs[platform]):
			lines.append("\t__{0} = (PFN_{0}) vkelGetDeviceProcAddr(device, \"{0}\");".format(func))
		
		if platform:
			lines.append("#endif /* " + platform + " */")
		
		lines.append("")
	
	lines.append("")
	
	
	lines.append("\t// Instance and device extension names")
	
	# Are there any (instance or device) extensions? (is array empty)
	if extension_names:
		for extension_name in sorted(extension_names):
			if extension_name:
				lines.append("\tVKEL_" + extension_name + " = vkelIsDeviceExtensionSupported(physicalDevice, NULL, \"VK_" + extension_name + "\");")
	
	lines.append("")
	
	
	lines.append("\t// Instance and device layer names")
	
	# Are there any (instance or device) layers? (is array empty)
	if layer_names:
		for layer_name in sorted(layer_names):
			if layer_name:
				lines.append("\tVKEL_" + layer_name + " = vkelIsDeviceLayerSupported(physicalDevice, \"VK_" + layer_name + "\");")
	
	lines.append("")
	
	
	f.write("\n".join(lines).encode("utf-8"))
	
	
	f.write(br'''
	
	return VK_TRUE;
}
	''')
	
	
	f.write(br'''
void vkelUninit(void)
{
	if (vkelVkLibHandle)
	{
		vkelPlatformCloseLibrary(vkelVkLibHandle);
		vkelVkLibHandle = NULL;
	}
}
	''')
	
	f.write(b"\n")
	
	
	f.write(br'''
#ifdef __cplusplus
}
#endif /* __cplusplus */''')


print("Done")
