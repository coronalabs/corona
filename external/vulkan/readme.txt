Lib32 and Include come from Vulkan SDK 1.2.162.1.

volk is available at https://github.com/zeux/volk and under the MIT license.

Vulkan Configurator was used to test with Vulkan's layers. It seems to refer to "Bin/VkLayer_khronos_validation.json", which in
turn refers to "VkLayer_khronos_validation.dll" by default as the "library_path". Since Solar is 32-bit we want to redirect this
to "..\\Bin32\\VkLayer_khronos_validation.dll". Other layers work likewise.

Also, an extension is used to flip the Vulkan viewport by using negative heights. However (possibly because so far only Vulkan 1.0
is used), the configurator will spew some warnings. In Debug we control the debug output and just suppress these, but will get
(harmless) reports in Release.

On that same note, we get some warnings about small dedicated resources. These are few in number and persist throughout a program
run, so are harmless. Ideally we could merge them into one memory resource, but each of them also gets memory-mapped so it wouldn't
be straightforward to do so.