#include <assert.h>

#include <iostream>
#include <vector>

#define _UNICODE

#ifdef _WIN32
#pragma comment(linker, "/subsystem:console")
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#ifndef NOMINMAX
#define NOMINMAX /* Don't let Windows define min() or max() */
#endif
#endif

#define APP_NAME_STR_LEN 80

#include <vulkan/vulkan.h>

// MS-Windows event handling function:
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // struct sample_info *info = reinterpret_cast<struct sample_info *>(GetWindowLongPtr(hWnd,
    // GWLP_USERDATA));

    switch (uMsg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            break;
        case WM_PAINT:
            // run(info);
            return 0;
        default:
            break;
    }
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

int main(int argc, char const *argv[]) {
    // Creating Instance
    //
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

    // Creating Device Queue
    //
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

    // Creating Command Buffer Pool
    //
    VkCommandPoolCreateInfo cmd_pool_info = {};
    cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_info.pNext = NULL;
    cmd_pool_info.queueFamilyIndex = 1;
    cmd_pool_info.flags = 0;

    VkCommandPool cmd_pool;
    result = vkCreateCommandPool(device, &cmd_pool_info, NULL, &cmd_pool);

    VkCommandBufferAllocateInfo cmd_info = {};
    cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_info.pNext = NULL;
    cmd_info.commandPool = cmd_pool;
    cmd_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_info.commandBufferCount = 1;

    VkCommandBuffer cmd;
    result = vkAllocateCommandBuffers(device, &cmd_info, &cmd);

    // Creating Surface
    //
    VkSurfaceKHR surface;
    std::vector<const char *> instance_extension_names;
    std::vector<const char *> device_extension_names;

    // load general and platform-specific surface extension
    instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef _WIN32
    instance_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
    assert("Unsupported platform", false);
#endif

    // load swapchain extension as a device extension
    device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

#ifdef _WIN32
    HINSTANCE connection;         // Windows instance
    char name[APP_NAME_STR_LEN];  // title for app
    HWND window;                  // window handle

    WNDCLASSEX win_class;

    connection = GetModuleHandle(NULL);
    sprintf_s(name, "Sample");

    int width = 1080;
    int height = 860;

    // Initialize the window class structure:
    win_class.cbSize = sizeof(WNDCLASSEX);
    win_class.style = CS_HREDRAW | CS_VREDRAW;
    win_class.lpfnWndProc = WndProc;
    win_class.cbClsExtra = 0;
    win_class.cbWndExtra = 0;
    win_class.hInstance = connection;  // hInstance
    win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    win_class.lpszMenuName = NULL;
    win_class.lpszClassName = name;
    win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
    // Register window class:
    if (!RegisterClassEx(&win_class)) {
        // It didn't work, so try to give a useful error:
        printf("Unexpected error trying to start the application!\n");
        fflush(stdout);
        exit(1);
    }
    // Create window with the registered class:
    RECT wr = {0, 0, width, height};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    window = CreateWindowEx(0,
                            name,                  // class name
                            name,                  // app name
                            WS_OVERLAPPEDWINDOW |  // window style
                                WS_VISIBLE | WS_SYSMENU,
                            100, 100,            // x/y coords
                            wr.right - wr.left,  // width
                            wr.bottom - wr.top,  // height
                            NULL,                // handle to parent
                            NULL,                // handle to menu
                            connection,          // hInstance
                            NULL);               // no extra parameters
    if (!window) {
        // It didn't work, so try to give a useful error:
        printf("Cannot create a window in which to draw!\n");
        fflush(stdout);
        exit(1);
    }

    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.hinstance = connection;
    createInfo.hwnd = window;
    result = vkCreateWin32SurfaceKHR(instance, &createInfo, NULL, &surface);
#else
    assert("Unsupported platform", false);
#endif  // _WIN32

    // Iterate over each queue and check if it supports presenting
    VkBool32 *pSupportsPresent = (VkBool32 *)malloc(queue_family_count * sizeof(VkBool32));
    for (uint32_t i = 0; i < queue_family_count; i++) {
        vkGetPhysicalDeviceSurfaceSupportKHR(gpus[0], i, surface, &pSupportsPresent[i]);
    }

    // Search for queue that supports both graphics and present
    size_t graphics_queue_family_index = UINT32_MAX;
    size_t present_queue_family_index = UINT32_MAX;
    for (uint32_t i = 0; i < queue_family_count; ++i) {
        if ((queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
            graphics_queue_family_index = i;
        if (pSupportsPresent[i] == VK_TRUE) {
            graphics_queue_family_index = i;
            present_queue_family_index = i;
            break;
        }
    }

    if (present_queue_family_index == UINT32_MAX) {
        // if a queue with both present and graphics wasn't found, find a seperate present queue
        for (size_t i = 0; i < queue_family_count; ++i) {
            if (pSupportsPresent[i] == VK_TRUE) {
                present_queue_family_index = i;
                break;
            }
        }
    }
    free(pSupportsPresent);

#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.pNext = NULL;
    create_info.hinstance = connection;
    create_info.hwnd = window;
    result = vkCreateWin32SurfaceKHR(instance, &create_info, NULL, &surface);
#endif

    VkSwapchainCreateInfoKHR swapchain_ci = {};
    swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_ci.pNext = NULL;
    swapchain_ci.surface = surface;
    swapchain_ci.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    // get more info needed to create swapchain
    VkSurfaceCapabilitiesKHR surface_capabilities;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpus[0], surface, &surface_capabilities);
    assert(("failed to set surface capabilities", result == VK_SUCCESS));
    uint32_t present_mode_count;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(gpus[0], surface, &present_mode_count, NULL);
    assert(("failed to count present modes", result == VK_SUCCESS));
    VkPresentModeKHR *present_modes =
        (VkPresentModeKHR *)malloc(present_mode_count * sizeof(VkPresentModeKHR));
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(gpus[0], surface, &present_mode_count,
                                                       present_modes);
    assert(("failed to get present modes", result == VK_SUCCESS));

    uint32_t desired_number_of_swapchain_images = surface_capabilities.minImageCount;

    // surface dimensions
    VkExtent2D swapchain_extent;
    // width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
    if (surface_capabilities.currentExtent.width == 0xFFFFFFFF) {
        // If the surface size is undefined, the size is set to
        // the size of the images requested.
        swapchain_extent.width = width;
        swapchain_extent.height = height;
        if (swapchain_extent.width < surface_capabilities.minImageExtent.width) {
            swapchain_extent.width = surface_capabilities.minImageExtent.width;
        } else if (swapchain_extent.width > surface_capabilities.maxImageExtent.width) {
            swapchain_extent.width = surface_capabilities.maxImageExtent.width;
        }

        if (swapchain_extent.height < surface_capabilities.minImageExtent.height) {
            swapchain_extent.height = surface_capabilities.minImageExtent.height;
        } else if (swapchain_extent.height > surface_capabilities.maxImageExtent.height) {
            swapchain_extent.height = surface_capabilities.maxImageExtent.height;
        }
    } else {
        // If the surface size is defined, the swap chain size must match
        swapchain_extent = surface_capabilities.currentExtent;
    }

    // The FIFO present mode is guaranteed by the spec to be supported
    VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;

    VkSurfaceTransformFlagBitsKHR pre_transform;
    if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        pre_transform = surface_capabilities.currentTransform;
    }

    swapchain_ci.minImageCount = desired_number_of_swapchain_images;
    swapchain_ci.imageExtent.width = swapchain_extent.width;
    swapchain_ci.presentMode = swapchain_present_mode;
    swapchain_ci.preTransform = pre_transform;

    vkDestroyInstance(instance, NULL);
    return 0;
}
