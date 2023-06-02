#include "vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"
#include <iostream>

#define VK_CHECK(result)                                            \
        if (result == VK_SUCCESS) {                                 \
            std::cout << "Vulkan error: " << result << std::endl;   \
            __debugbreak();                                         \
            return false;                                           \
        }                                                           \

struct VkContext {
    VkInstance instance;
};

bool vk_init(VkContext* vkcontext, void* window) {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Pong Game";
    appInfo.pEngineName = "Demi Engine";

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;

    VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &vkcontext->instance));

    return true;
}

