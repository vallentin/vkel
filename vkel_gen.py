#!/usr/bin/python

# Allow Python 2.6+ to use the print() function
from __future__ import print_function

COPYRIGHT = br'''//========================================================================
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

'''



# Try to import Python 3 library urllib.request
# and if it fails, fall back to Python 2 urllib2
try:
	import urllib.request as urllib2
except ImportError:
	import urllib2

import os
import re



if not os.path.exists("vulkan.h"):
	print("Downloading vulkan.h from GitHub")
	print("URL: https://github.com/KhronosGroup/Vulkan-Docs/tree/1.0/src/vulkan")
	
	vulkan_h_url = "http://raw.githubusercontent.com/KhronosGroup/Vulkan-Docs/1.0/src/vulkan/vulkan.h"
	vulkan_h_web = urllib2.urlopen(vulkan_h_url)
	
	with open("vulkan.h", "wb") as f:
		f.writelines(vulkan_h_web.readlines())
else:
	print("Reusing existing vulkan.h")



if not os.path.exists("vk_platform.h"):
	print("Downloading vk_platform.h from GitHub")
	print("URL: https://github.com/KhronosGroup/Vulkan-Docs/tree/1.0/src/vulkan")
	
	vk_platform_h_url = "https://raw.githubusercontent.com/KhronosGroup/Vulkan-Docs/1.0/src/vulkan/vk_platform.h"
	vk_platform_h_web = urllib2.urlopen(vk_platform_h_url)
	
	with open("vk_platform.h", "wb") as f:
		f.writelines(vk_platform_h_web.readlines())



print("Parsing vulkan.h")


# all the functions no matter the platform
all_funcs = []

# all the function but platform specific
platform_funcs = {
	"": [] # none-platform specific functions
}

# current platform
platform = None

extensions = []


# regex_function = re.compile(r"PFN_(\w+)")
regex_function = re.compile(r"VKAPI_PTR\s+\*PFN_(\w+)")

regex_platform_begin = re.compile(r"#ifdef\s+(VK_USE_PLATFORM\w+)")
regex_platform_end = re.compile(r"#endif\s+/\*\s*(\w+)\s*\*/")

regex_extension = re.compile(r"#define\s+\w+_EXTENSION_NAME\s+\"VK_(\w+)\"")


with open("vulkan.h", "r") as f:
	for line in f:
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
		
		
		match_extension = regex_extension.search(line)
		
		if match_extension:
			extension = match_extension.group(1)
			
			if extension not in extensions:
				extensions.append(extension)




H_HEADER = br'''#pragma once
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
'''

H_BODY = br'''
PFN_vkVoidFunction vkelGetProcAddr(const char *name);
PFN_vkVoidFunction vkelGetInstanceProcAddr(VkInstance instance, const char *pName);

VkBool32 vkelIsSupported(const char *extension);

VkBool32 vkelInit(void);
void vkelUninit(void);
'''

H_FOOTER = br'''
#ifdef __cplusplus
}
#endif

#endif /* _vkel_h_ */'''



C_HEADER = br'''#include "vkel.h"

#ifdef __cplusplus
extern "C" {
#endif

// Required for calloc() and free()
#include <stdlib.h>
'''

C_BODY = br'''
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
'''

C_FOOTER = br'''
#ifdef __cplusplus
}
#endif'''



print("Generating vkel.h")

with open("vkel.h", "wb") as f:
	f.write(COPYRIGHT)
	f.write(H_HEADER)
	
	
	lines = []
	
	lines.append("")
	
	for extension in extensions:
		lines.append("VkBool32 VKEL_" + extension + ";")
	
	lines.append("")
	
	f.write("\n".join(lines).encode("utf-8"))
	
	
	f.write(H_BODY)
	
	
	lines = []
	
	lines.append("")
	
	for platform, funcs in platform_funcs.items():
		if platform:
			for func in funcs:
				lines.append("#define {0} NULL".format(func))
			
			lines.append("")
			lines.append("#ifdef " + platform)
		
		for func in funcs:
			# lines.append("PFN_{0} {0};".format(func))
			lines.append("PFN_{0} __{0};".format(func))
			
			if platform:
				lines.append("#undef {0}".format(func))
			
			lines.append("#define {0} __{0}".format(func))
		
		if platform:
			lines.append("#endif /* " + platform + " */")
		
		lines.append("")
	
	# lines.append("")
	
	f.write("\n".join(lines).encode("utf-8"))
	
	
	f.write(H_FOOTER)



print("Generating vkel.c")

with open("vkel.c", "wb") as f:
	f.write(COPYRIGHT)
	f.write(C_HEADER)
	f.write(C_BODY)
	
	
	f.write(br'''
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
''')
	
	
	lines = []
	
	lines.append("")
	
	for platform, funcs in platform_funcs.items():
		if platform:
			lines.append("#ifdef " + platform)
		
		for func in funcs:
			# lines.append("PFN_{0} {0};".format(func))
			# lines.append("PFN_{0} __{0};".format(func))
			# lines.append("#define {0} __{0}".format(func))
			lines.append("\t__{0} = (PFN_{0}) vkelGetProcAddr(\"{0}\");".format(func))
		
		if platform:
			lines.append("#endif /* " + platform + " */")
		
		lines.append("")
	
	for extension in extensions:
		lines.append("\tVKEL_" + extension + " = vkelIsSupported(\"VK_" + extension + "\");")
	
	lines.append("")
	# lines.append("")
	
	f.write("\n".join(lines).encode("utf-8"))
	
	
	f.write(br'''
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
''')
	
	
	f.write(C_FOOTER)



print("Done")
