#include <assert.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>

int main(int argc, char const *argv[]) {
    VkInstance instance;
    VkResult result;

    VkApplicationInfo application_info = {};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pNext = NULL;
    application_info.pApplicationName = "Application Name";
    application_info.applicationVersion = 0;
    application_info.pEngineName = "EngineName";
    application_info.engineVersion = 0;
    application_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instance_info = {};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pNext = NULL;
    instance_info.flags = 0;
    instance_info.pApplicationInfo = &application_info;
    instance_info.enabledExtensionCount = 0;
    instance_info.ppEnabledExtensionNames = NULL;
    instance_info.enabledLayerCount = 0;

    result = vkCreateInstance(&instance_info, NULL, &instance);  // 0 if successful
    if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
        std::cout << "cannot find a compatible Vulkan Installable Client Driver\n";
        exit(-1);
    } else if (result) {
        std::cout << "unknown error\n";
        exit(-1);
    }

    VkDeviceQueueCreateInfo queue_info = {};

    uint32_t gpu_count;
    result = vkEnumeratePhysicalDevices(instance, &gpu_count,
                                        NULL);  // API writes found GPU count to &gpu_count
    assert(("No devices found", gpu_count > 0));
    std::vector<VkPhysicalDevice> gpus;
    gpus.resize(gpu_count);  // allocate space for all GPUs found
    result = vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data());

    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queue_family_count, NULL);
    assert(("No queue families", queue_family_count > 0));
    std::vector<VkQueueFamilyProperties> queue_properties;
    queue_properties.resize(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queue_family_count, queue_properties.data());
    assert(("No queue families", queue_family_count > 0));

    // look for only graphics bit
    bool found = false;
    for (unsigned int i = 0; i < queue_family_count; i++) {
        if (queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queue_info.queueFamilyIndex = i;
            found = true;
            break;
        }
    }

    float queue_priorities[1] = {0.0};  // unimportant because only 1 queue
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = NULL;
    queue_info.queueCount = 1;  // add more as needed
    queue_info.pQueuePriorities = queue_priorities;

    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.pNext = NULL;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &queue_info;
    device_info.enabledExtensionCount = 0;
    device_info.ppEnabledExtensionNames = NULL;
    device_info.enabledLayerCount = 0;  // layers are deprecated
    device_info.ppEnabledLayerNames = NULL;
    device_info.pEnabledFeatures = NULL;

    VkDevice device;
    result = vkCreateDevice(gpus[0], &device_info, NULL, &device);
    assert(("Error creating device", result == VK_SUCCESS));

// vkCreateCommandPool


    vkDestroyInstance(instance, NULL);
    return 0;
}
