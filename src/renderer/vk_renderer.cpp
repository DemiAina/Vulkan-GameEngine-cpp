#include "vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"

struct VkContext {
    VkInstance instance;
};

bool vk_init(VkContext* vkcontext){
    
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Pong Game";
    appInfo.pEngineName = "Demi Engine";

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    
    VkResult result = vkCreateInstance(&instanceInfo, 0 , &vkcontext->instance);
    
    if(result == VK_SUCCESS){
        return true;
    }
    return false;
}
