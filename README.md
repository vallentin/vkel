# vkel - Simple Vulkan Extension Loader

*Simple Dynamic Cross-Platform Vulkan Extension Loader*

*Updated for [Vulkan 1.0.8](https://www.khronos.org/registry/vulkan/#apispecs).*


#### Vulkan Documentation

- [Vulkan 1.0.8 Specification](https://www.khronos.org/registry/vulkan/specs/1.0/xhtml/vkspec.html)
- [Vulkan 1.0.8 + WSI Extension Specification](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html)


#### Specification Updates

- [Vulkan 1.0.8 Specification Update](https://github.com/KhronosGroup/Vulkan-Docs/commit/ce3204c7dd53a0116ce3a91fd12919d606c306f5)
- [Vulkan 1.0.7 Specification Update](https://github.com/KhronosGroup/Vulkan-Docs/commit/8c3c9b4c85f2539b67148c2de9e2573154c92786)
- [Vulkan 1.0.6 Specification Update](https://github.com/KhronosGroup/Vulkan-Docs/commit/1f875738fd9bd0aca5d36bf36794b4b2903e74a9)
- [Vulkan 1.0.5 Specification Update](https://github.com/KhronosGroup/Vulkan-Docs/commit/7380aee56b53e9ea3647186ec477202f7f17e28c)
- [Vulkan 1.0.4 Specification Update](https://github.com/KhronosGroup/Vulkan-Docs/commit/5a4c5e5925c65c6e6677c1fb21571684b4b0a77b)


## Introduction

[vkel][vkel] is a simple and easy way of dynamically loading
Vulkan and its function pointers.
[vkel][vkel] can also be used to check which (instance or device)
extensions and layers are available.

*Also yes, the idea of [vkel][vkel] is indeed
based on [GLEW](https://github.com/nigels-com/glew) and [gl3w](https://github.com/skaslev/gl3w).*

**Notice:** ~~Your driver might not support the newest released patch, so instead of using
`VK_API_VERSION` use something like `VK_MAKE_VERSION(1, 0, 3)`.~~ In Vulkan 1.0.6 the
`VK_API_VERSION` macro was deprecated and removed, in favor of using `VK_MAKE_VERSION(1, 0, 6)`.

## Setup

[vkel][vkel] doesn't (any more) require the vulkan headers.
So simply download [vkel.h][vkel.h] and
[vkel.c][vkel.c] and you're done!
*Do note that this takes into account, that the Vulkan library and driver exists on the system.*

### Unix-like OS

On Unix-like OS' you might have to build using `-fPIC` (Position Independent Code).

Example: `gcc -Wall -g -fPIC -shared -o libvkel.so vkel.c`

*Before the building problem on UNIX was encountered, someone responded that he/she
was using `gcc -std=c99 -fPIC -shared -o libvkel.so vkel.c` when building.*

## Generating

Overall the only thing needed is [vkel_gen.py][vkel_gen.py].
When executing the script will temporarily download `vulkan.h` and `vk_platform.h`. Though if they already
exist within the same folder as [vkel_gen.py][vkel_gen.py],
then it will fallback to using them. Be aware that when new versions of the Vulkan headers are available,
this can create problems.

*There's also two flags you can use when executing the script:*

- `-f`: force, always download `vulkan.h` and `vk_platform.h`
- `-k`: keep, save `vulkan.h` and `vk_platform.h`

*Note: `vkel_gen.py` is developed and tested using Python 3.5.1.*

## Example: Functionality

Vulkan does not have a context unlike OpenGL. You create a Vulkan instance, and compared
to OpenGL the Vulkan instance is created via the Vulkan API itself.
*In other words `vkelInit()` can be called as the first thing, without needing anything else beforehand!*

Though along the way function pointers are needed according to the created `VkInstance` and `VkDevice`,
for that simply use `vkelInstanceInit()` and `vkelDeviceInit()` to reload function pointers and re-check
support for extensions and layers. This is all done relative to the given instance or device.

```c
#include <stdio.h> // needed for fprintf()

#include "vkel.h"

int main(int argc, char **argv)
{
	// vkelInit() returns VK_TRUE on success or VK_FALSE on failing
	// to load the Vulkan library (meaning that the library is most
	// likely missing).
	if (!vkelInit())
	{
		fprintf(stderr, "Failed to initialize Vulkan\n");
		return -1;
	}
	
	
	VkInstance instance;
	// Do all the stuff to create a VkInstance
	
	if (!vkelInstanceInit(instance))
	{
		fprintf(stderr, "Failed to initialize Vulkan with VkInstance\n");
		return -1;
	}
	
	
	VkDevice device;
	// Do all the stuff to create a VkDevice
	
	if (!vkelDeviceInit(device))
	{
		fprintf(stderr, "Failed to initialize Vulkan with VkDevice\n");
		return -1;
	}
	
	
	// Static checking is checked on vkelInit() where dynamic checking
	// always are checked directly against Vulkan (making the dynamic
	// version slower).
	
	// Statically check if the extension is supported
	if (VKEL_KHR_android_surface)
	{
		// The extension is supported
	}
	
	// Statically check if the extension is supported
	if (VKEL_NV_glsl_shader)
	{
		// The extension is supported
	}
	
	
	// Dynamically check if the extension is supported
	if (vkelIsInstanceExtensionSupported(NULL, "VK_KHR_android_surface"))
	{
		// The extension is supported
	}
	
	
	// Check if the function is supported
	if (vkCreateAndroidSurfaceKHR)
	{
		// The function is indeed supported
	}
	
	
	// All functions found in vulkan.h are automatically loaded making vkelGetProcAddr()
	// unneeded unless functions are added which aren't added to vulkan.h
	// or when (if ever) using a modified Vulkan library.
	
	// Manually load function
	PFN_vkQueuePresentKHR pfnQueuePresentKHR = (PFN_vkQueuePresentKHR) vkelGetProcAddr("vkQueuePresentKHR");
	PFN_vkQueuePresentKHR pfnQueuePresentKHR = (PFN_vkQueuePresentKHR) vkelGetInstanceProcAddr(instance, "vkQueuePresentKHR");
	PFN_vkQueuePresentKHR pfnQueuePresentKHR = (PFN_vkQueuePresentKHR) vkelGetDeviceProcAddr(device, "vkQueuePresentKHR");
	
	
	// Release the Vulkan library again (the OS will also do this automatically of course).
	vkelUninit();
	
	
	return 0;
}
```

## Example: List Supported Extensions

[vkel][vkel] can also be used to list all
supported extensions and layers using:

- `vkelGetInstanceExtensionNames`
- `vkelGetInstanceLayerNames`
- `vkelGetDeviceExtensionNames`
- `vkelGetDeviceLayerNames`

Individual extensions and layers can be checked using:

- `vkelIsInstanceLayerSupported`
- `vkelIsInstanceExtensionSupported`
- `vkelIsDeviceLayerSupported`
- `vkelIsDeviceExtensionSupported`

```c
#include <stdio.h> // needed for getchar(), printf() and fprintf()
#include <stdlib> // needed for calloc() and free()

#include "vkel.h"

int main(int argc, char **argv)
{
	if (!vkelInit())
	{
		fprintf(stderr, "Failed to initialize Vulkan\n");
		return -1;
	}
	
	
	uint32_t extensionNameCount = 0;
	char **extensionNames = vkelGetInstanceExtensionNames(NULL, &extensionNameCount);
	
	printf("Count: %d\n", extensionNameCount);
	
	for (uint32_t extensionNameIndex = 0; extensionNameIndex < extensionNameCount; extensionNameIndex++)
	{
		printf("Extension %d: %s\n", (extensionNameIndex + 1), extensionNames[extensionNameIndex]);
	}
	
	printf("\n");
	
	vkelDeleteInstanceExtensionNames(extensionNameCount, extensionNames);
	extensionNames = NULL;
	
	
	// Pause, so we get to see something before it exits
	getchar();
	
	
	// Release the Vulkan library again (the OS will also do this automatically of course).
	vkelUninit();
	
	
	return 0;
}
```

## API Reference


### Initialize

`VkBool32 vkelInit(void)`
> Initialize and load Vulkan along with the function pointers. Returns `VK_TRUE` when vkel was
> initialized successfully and `VK_FALSE` if the Vulkan library couldn't be loaded (most likely
> meaning that the library is missing).

`VkBool32 vkelInstanceInit(VkInstance instance)`
> Reload function pointers according to the given `VkInstance`. This also re-checks
> support for extensions and layers.

`VkBool32 vkelDeviceInit(VkPhysicalDevice physicalDevice, VkDevice device)`
> Reload function pointers according to the given `VkDevice`. This also re-checks
> support for extensions and layers, using the given `VkPhysicalDevice`.

`void vkelUninit(void)`
> Free the Vulkan library (the OS will do this automatically if `vkelUninit()` isn't called).

### Function Pointers

`PFN_vkVoidFunction vkelGetProcAddr(const char *name)`
> Get a function pointer from the loaded Vulkan library.

`PFN_vkVoidFunction vkelGetInstanceProcAddr(VkInstance instance, const char *pName)`
> Shortcut for calling `vkGetInstanceProcAddr()`, but if it returns NULL, then call `vkelGetProcAddr()`.

`PFN_vkVoidFunction vkelGetDeviceProcAddr(VkDevice device, const char *pName)`
> Shortcut for calling `vkGetDeviceProcAddr()`, but if it returns NULL, then call `vkelGetInstanceProcAddr()`.


### Check Supported Extensions/Layers

- `VkBool32 vkelIsInstanceLayerSupported(const char *pLayerName)`
- `VkBool32 vkelIsDeviceLayerSupported(VkPhysicalDevice physicalDevice, const char *pLayerName)`

> Check if instance/device layer is supported.

- `VkBool32 vkelIsInstanceExtensionSupported(const char *pLayerName, const char *pExtensionName)`
- `VkBool32 vkelIsDeviceExtensionSupported(VkPhysicalDevice physicalDevice, const char *pLayerName, const char *pExtensionName)`

> Check if instance/device extension is supported.

*Remember that they can be checking using the extension name itself. With the
minor change of having the prefix `VKEL_` instead of `VK_`. Example, `VK_KHR_win32_surface` would
be `VKEL_KHR_win32_surface`.*


### Listing Supported Extensions/Layers

*Check the example above.*

- `char** vkelGetInstanceExtensionNames(const char *pLayerName, uint32_t *extensionNameCount)`
- `char** vkelGetDeviceExtensionNames(VkPhysicalDevice physicalDevice, const char *pLayerName, uint32_t *extensionNameCount)`

> Get an array of all the supported instance/device extension names.

- `char** vkelGetInstanceLayerNames(uint32_t *layerNameCount)`
- `char** vkelGetDeviceLayerNames(VkPhysicalDevice physicalDevice, uint32_t *layerNameCount)`

> Get an array of all the supported instance/device layer names.

- `void vkelDeleteInstanceExtensionNames(uint32_t extensionNameCount, char **extensionNames)`
- `void vkelDeleteInstanceLayerNames(uint32_t layerNameCount, char **layerNames)`
- `void vkelDeleteDeviceExtensionNames(uint32_t extensionNameCount, char **extensionNames)`
- `void vkelDeleteDeviceLayerNames(uint32_t layerNameCount, char **layerNames)`

> The return `char**` can be manually deleted, but the above function exist for simplifying
> the process. The above functions are also just `#define`'s of `vkelDeleteNames()`


## Reporting Bugs & Requests

Feel free to use the [issue tracker](https://github.com/MrVallentin/vkel/issues).
Please always include the name and version of the OS where the bug occurs.

*I don't have the means to test this on all the different OS'. So any confirmation would be much obliged.*

If you have a solution, then feel free to fork it as well and the changes will be included.


## Dependencies

- Vulkan - *If you're using this in the young days of Vulkan, then make sure that you have the Vulkan driver installed, if any problems occur.*
- Windows (header) - needed for library loading on Windows
- dlfcn (header) - needed for library loading on non-Windows OS'
- Standard C Libraries (stdio, stdlib, string, assert) - needed for NULL, malloc() calloc(), free(), memset(), assert()


### License & Copyright

```
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would
   be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not
   be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
   distribution.
```

#### Additional Copyright

Vulkan™ and the Vulkan logo are trademarks of the Khronos Group Inc.

![Vulkan Logo](http://advvulkan.com/Vulkan_500px_Mar15.png)


[vkel]: https://github.com/MrVallentin/vkel
[vkel.h]: https://github.com/MrVallentin/vkel/blob/master/vkel.h
[vkel.c]: https://github.com/MrVallentin/vkel/blob/master/vkel.c
[vkel_gen.py]: https://github.com/MrVallentin/vkel/blob/master/vkel_gen.py
