#include "pch.h"

namespace vk
{
    VulkanManager::VulkanManager(GLFWwindow* window)
    {
        CreateInstance();
        GetPhysicalDeviceAndQueuesFamilies();
        CreateSurface(window);
        CreateDevice();
    }

    VulkanManager::~VulkanManager()
    {
        vkDestroyDevice(device_, nullptr);
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        vkDestroyInstance(instance_, nullptr);
    }

    void VulkanManager::CreateInstance()
    {
        // create app info
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Vulkan Demo 2";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;

        // get glfw extensions
        uint32_t glfw_extension_count = 0;
        const char** glfw_extensions;
        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        // create info
        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = glfw_extension_count;
        create_info.ppEnabledExtensionNames = glfw_extensions;
        create_info.enabledLayerCount = 0;

        // create instance
        if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create instance.");
    }

    void VulkanManager::GetPhysicalDeviceAndQueuesFamilies()
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);

        std::cout << "Graphics Card Count: " << device_count << std::endl;

        if (device_count == 0)
            throw std::runtime_error("Failed to get physical device.");

        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());

        // get first graphics card
        physical_device_ = devices[0];
        if (physical_device_ == VK_NULL_HANDLE)
            throw std::runtime_error("Failed to get physical device.");

        // get queues families
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queue_family_count, nullptr);

        queue_families_.resize(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queue_family_count, queue_families_.data());
    }

    void VulkanManager::CreateDevice()
    {
        // get graphics queue family index
        uint32_t graphics_queue_family_index = -1;
        for (int i = 0; i < queue_families_.size(); i++)
        {
            if (queue_families_[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                graphics_queue_family_index = i;
                break;
            }
        }

        // get present queue family index
        uint32_t present_queue_family_index = -1;
        for (int i = 0; i < queue_families_.size(); i++)
        {
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, i, surface_, &present_support);

            if (present_support)
            {
                present_queue_family_index = i;
                break;
            }
        }

        // create device queue create infos
        std::vector<uint32_t> queue_family_indices = { graphics_queue_family_index, present_queue_family_index };
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        float queue_priority = 1.0f;
        for (auto index : queue_family_indices)
        {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = index;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(queue_create_info);
        }

        // create logical device
        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        create_info.enabledLayerCount = 0;

        if (vkCreateDevice(physical_device_, &create_info, nullptr, &device_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create logical device.");
    }

    void VulkanManager::CreateSurface(GLFWwindow* window)
    {
        if (glfwCreateWindowSurface(instance_, window, nullptr, &surface_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create window surface.");
    }
}
