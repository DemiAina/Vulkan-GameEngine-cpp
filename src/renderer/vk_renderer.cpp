#include <windows.h>
#include "vulkan/vulkan.h"
#ifdef WINDOW_BUILD
#include "vulkan/vulkan_win32.h"
#elif LINUX_BUILD
#endif

#include "vulkan/vulkan_core.h"
#include <iostream>

#define ArraySize(arr) sizeof((arr)) / sizeof((arr[0]))

#define VK_CHECK(result)                                            \
        if (result != VK_SUCCESS) {                                 \
            std::cout << "Vulkan error: " << result << std::endl;   \
            __debugbreak();                                         \
            return false;                                           \
        }                                                           \

static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
    VkDebugUtilsMessageTypeFlagsEXT msgFlags,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    std::cout << "Validation Error: " << pCallbackData->pMessage << std::endl;
    return false;
}

struct VkContext {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surfaceFormat;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkSwapchainKHR swapchain;
    VkDebugUtilsMessengerEXT debugMessenger;
    int graphicsIdx;
};

bool vk_init(VkContext* vkcontext, void* window) {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Pong Game";
    appInfo.pEngineName = "Demi Engine";
    
    char* extensions[] = {
#ifdef WINDOW_BUILD
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif   LINUX_BUILD
#endif
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME
    };

    char *layers[]{

        "VK_LAYER_KHRONOS_validation"
    };

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.ppEnabledExtensionNames = extensions;
    instanceInfo.enabledExtensionCount = ArraySize(extensions);
    instanceInfo.ppEnabledLayerNames = layers;
    instanceInfo.enabledLayerCount = ArraySize(layers);
    VK_CHECK(vkCreateInstance(&instanceInfo, 0, &vkcontext->instance))

   auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkcontext->instance, "vkCreateDebugUtilsMessengerEXT");
    if (vkCreateDebugUtilsMessengerEXT)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debugInfo.pfnUserCallback = vk_debug_callback;

        vkCreateDebugUtilsMessengerEXT(vkcontext->instance, &debugInfo, 0, &vkcontext->debugMessenger);    
    }
    else
    {
        return false;
    }
    //Create surface
    {
#ifdef WINDOW_BUILD
    VK_CHECK(vkCreateInstance(&instanceInfo, 0, &vkcontext->instance));

    VkWin32SurfaceCreateInfoKHR surfaceInfo = {};

    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR; 
    surfaceInfo.hwnd = (HWND)window;
    surfaceInfo.hinstance = GetModuleHandleA(0);
    VK_CHECK(vkCreateWin32SurfaceKHR(vkcontext->instance, &surfaceInfo, 0 , &vkcontext->surface));
#elif LINUX_BUILD
#endif 
    }

    // Create gpu
    {
        vkcontext->graphicsIdx = -1;
        uint32_t gpuCount = 0;
        //TODO: Suballocation from Main Allocation 
        VkPhysicalDevice gpus[10];
        VK_CHECK(vkEnumeratePhysicalDevices(vkcontext->instance, &gpuCount, 0));
        VK_CHECK(vkEnumeratePhysicalDevices(vkcontext->instance, &gpuCount, 0));
        
        for(uint32_t i = 0 ; i < gpuCount; i++){
            VkPhysicalDevice gpu = gpus[i];
            uint32_t queueFamilyCount = 0;
            //TODO: Suballocation from Main Allocation 
            VkQueueFamilyProperties queueProps[10];
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, 0);
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueProps);

            for(uint32_t j = 0; j < queueFamilyCount; j++){
                if(queueProps[j].queueFlags & VK_QUEUE_COMPUTE_BIT){
                    VkBool32 surfaceSupport = VK_FALSE;
                    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, j , vkcontext->surface, &surfaceSupport));
                    if(surfaceSupport){
                        vkcontext->graphicsIdx = j;
                        vkcontext->gpu = gpu;
                        break;
                    }
                }
            }
        }
        if(vkcontext->graphicsIdx < 0){
            return false;
        }
    }


    //Logical Device

    {
        float queuePriortity = 1.0f;
        VkDeviceQueueCreateInfo queueInfo = {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = vkcontext->graphicsIdx;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriortity;

        char *extensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME}; 

        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.pQueueCreateInfos = &queueInfo;
        deviceInfo.queueCreateInfoCount = 1;
        deviceInfo.ppEnabledExtensionNames = extensions;
        deviceInfo.enabledExtensionCount = ArraySize(extensions);
        VK_CHECK(vkCreateDevice(vkcontext->gpu, &deviceInfo, 0  , &vkcontext->device));
    }
     // SwapChain
    {
        uint32_t formatCount = 0;
        //TODO: Sub memory allocation
        VkSurfaceFormatKHR surfaceFormats[10];

        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vkcontext->gpu, vkcontext->surface, &formatCount, 0));
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vkcontext->gpu, vkcontext->surface, &formatCount, surfaceFormats));

        for (uint32_t i = 0; i < formatCount; i++) {
            VkSurfaceFormatKHR format = surfaceFormats[i]; 
                if(format.format == VK_FORMAT_B8G8R8A8_SRGB){
                    vkcontext->surfaceFormat = format;
                    break;
                }
        }

        VkSurfaceCapabilitiesKHR surfaceCaps = {};
        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkcontext->gpu,vkcontext->surface, &surfaceCaps));
        uint32_t imgCount = surfaceCaps.minImageCount + 1;
        imgCount = imgCount > surfaceCaps.minImageCount ? imgCount - 1: imgCount;
        
        VkSwapchainCreateInfoKHR scInfo = {};
        scInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        scInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // This can result in confusion (come back to this)
        scInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        scInfo.imageFormat = vkcontext->surfaceFormat.format;
        scInfo.surface = vkcontext->surface;
        scInfo.preTransform = surfaceCaps.currentTransform;
        scInfo.imageExtent = surfaceCaps.currentExtent;
        scInfo.minImageCount = imgCount;
        scInfo.imageArrayLayers = 1;
        VK_CHECK(vkCreateSwapchainKHR(vkcontext->device, &scInfo, 0, &vkcontext->swapchain));

    }


    return true;
}
