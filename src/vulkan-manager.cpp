#include "pch.h"

namespace vk
{
    VulkanManager::VulkanManager()
    {
        CreateInstance();
        GetPhysicalDeviceAndQueuesFamilies();
        CreateDevice();
    }

    VulkanManager::~VulkanManager()
    {
        vkDestroyDevice(device_, nullptr);
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
        int i;
        for (i = 0; i < queue_families_.size(); i++)
            if (queue_families_[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                break;

        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = i;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = queue_priorities_.data();

        VkPhysicalDeviceFeatures device_features{};

        // create logical device
        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.pQueueCreateInfos = &queue_create_info;
        create_info.queueCreateInfoCount = 1;
        create_info.pEnabledFeatures = &device_features;
        create_info.enabledLayerCount = 0;

        if (vkCreateDevice(physical_device_, &create_info, nullptr, &device_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create logical device.");
    }
}
