# vkel - Simple Vulkan Extension Loader

*Simple Dynamic Cross-Platform Vulkan Extension Loader*

## Introduction

[vkel](https://github.com/VallentinSource/vkel) is a simple and easy way of dynamically loading
Vulkan and its function pointers.

*Also yes, the idea of [vkel](https://github.com/VallentinSource/vkel) is indeed
based on [GLEW](https://github.com/nigels-com/glew) and [gl3w](https://github.com/skaslev/gl3w).*

## Setup

First of all either add `vkel.h` and `vkel.c` to the folder that contains `vulkan.h`,
or open `vkel.h` and edit `#include "vulkan.h"` so it's the correct path to `vulkan.h`.

On Unix-like OS' use `gcc -std=c99 -fPIC -shared -o libvkel.so vkel.c` when building.

After this simply `#include "vkel.h"` and everything should be working. *Do note that this takes
into account, that the Vulkan library and driver exists on the system.*

## Generating

The only thing needed is `vkel_gen.py`. When executing this Python script it will download `vulkan.h`, `vk_platform.h`,
from the official [Khronos Group Vulkan repository](https://github.com/KhronosGroup/Vulkan-Docs). Then it parses `vulkan.h`
and generates `vkel.h` and `vkel.c`. All these files are stored within the same directory as `vkel_gen.py`.

*Note: `vkel_gen.py` is developed and tested using Python 3.5.1.*

## Example: Functionality

Vulkan does not have a context unlike OpenGL. You create a Vulkan instance, and compared
to OpenGL the Vulkan instance is created via the Vulkan API itself.
*In other words `vkelInit()` can be called as the first thing, without needing anything else beforehand!*

```c
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
	
	
	// Static checking is checked on vkelInit() where dynamic checking
	// always are checked directly against Vulkan (making the dynamic
	// version slower).
	
	// Statically check if the extension is supported
	if (VKEL_KHR_android_surface)
	{
		// The extension is supported
	}
	
	// Dynamically check if the extension is supported
	if (vkelIsSupported("VK_KHR_android_surface"))
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
	PFN_vkQueuePresentKHR fpvkQueuePresentKHR = (PFN_vkQueuePresentKHR) vkelGetProcAddr("vkQueuePresentKHR");
	
	
	// Release the Vulkan library again (the OS will also do this automatically of course).
	vkelUninit();
	
	
	return 0;
}
```

## Example: List Supported Extensions

*This example isn't anything special to VKEL, but it shows which extensions the system supports.*

```c
#include <stdio.h> // needed for getchar() and printf()
#include <stdlib> // needed for calloc() and free()

#include "vkel.h"

int main(int argc, char **argv)
{
	if (!vkelInit())
	{
		fprintf(stderr, "Failed to initialize Vulkan\n");
    	return -1;
	}
	
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
				printf("%s  \t extension revision: %d\n", ext_props[i].extensionName, ext_props[i].specVersion);
			}
		}

		free(ext_props);
	}
	
	// Pause, so we get to see something before it exits
	getchar();
	
	
	// Release the Vulkan library again (the OS will also do this automatically of course).
	vkelUninit();
	
	
	return 0;
}
```

## API Reference

// Initialize the library once after a Vulkan context has been created.
// Returns 0 when vkel was initialized successfully, -1 if there was an error.

`VkBool32 vkelInit()`
> Initialize and load Vulkan along with the function pointers.
> Returns `VK_TRUE` when vkel was initialized successfully and `VK_FALSE` if
> the Vulkan library couldn't be loaded (most likely meaning that the
> library is missing).

`PFN_vkVoidFunction vkelGetProcAddr(const char *name)`
> Get a function pointer from the loaded Vulkan library.

`PFN_vkVoidFunction vkelGetInstanceProcAddr(VkInstance instance, const char *pName)`
> Shortcut for `vkGetInstanceProcAddr()` and if that returns NULL, then
> `vkelGetProcAddr()`.

`vkelIsSupported(const char *extension)`
> Check if the current system supports a given Vulkan extension.
> Use `vkelIsSupported("VK_KHR_android_surface")` for dynamic checking or
> `if (VKEL_KHR_android_surface)` for static checking. The difference being
> that the static version is checked when `vkelInit()` is called, and the dynamic
> version is checked again the Vulkan API every time.

`void vkelUninit()`
> Free the Vulkan library (the OS will do this automatically if `vkelUninit()` isn't called).


## Reporting Bugs & Requests

Feel free to use the [issue tracker](https://github.com/VallentinSource/vkel/issues).
Please always include the name and version of the OS where the bug occurs.

*I don't have the means to test this on the different OS'. So any confirmation would be much obliged.*

If you have a solution, then feel free to fork it as well and the changes will be included.


## Dependencies

- Vulkan - *If you're using this in the young days of Vulkan, then make sure that you have the Vulkan driver installed, if any problems occur.*
- stdlib.h (Standard C library) - needed for calloc() and free()


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
```

#### Additional Copyright

Vulkan™ and the Vulkan logo are trademarks of the Khronos Group Inc.

![Vulkan Logo](http://advvulkan.com/Vulkan_500px_Mar15.png)
