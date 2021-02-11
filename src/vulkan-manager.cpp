#include "pch.h"

namespace vk
{
    VulkanManager::VulkanManager(GLFWwindow* window, uint32_t width, uint32_t height)
    {
        CreateInstance();
        CreateSurface(window);
        GetPhysicalDeviceAndQueuesFamilies();
        CreateDevice();
        CreateSwapchain(width, height);
    }

    VulkanManager::~VulkanManager()
    {
        for (auto image_view : swapchain_image_views_)
            vkDestroyImageView(device_, image_view, nullptr);
        vkDestroySwapchainKHR(device_, swapchain_, nullptr);
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
#ifdef _DEBUG
        std::vector<const char*> validation_layers =
        {
            "VK_LAYER_KHRONOS_validation"
        };
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
#else
        create_info.enabledLayerCount = 0;
#endif

        // create instance
        if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create instance.");
    }

    void VulkanManager::CreateSurface(GLFWwindow* window)
    {
        if (glfwCreateWindowSurface(instance_, window, nullptr, &surface_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create window surface.");
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

        // get graphics queue family index
        bool found = false;
        for (int i = 0; i < queue_families_.size(); i++)
        {
            if (queue_families_[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                // graphics queue family index
                using_queue_family_indices_.insert(i);
                found = true;
                break;
            }
        }
        if (!found) throw std::runtime_error("Device does not support graphics queue.");

        // get present queue family index
        found = false;
        for (int i = 0; i < queue_families_.size(); i++)
        {
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, i, surface_, &present_support);

            if (present_support)
            {
                // present queue family index
                using_queue_family_indices_.insert(i);
                found = true;
                break;
            }
        }
        if (!found) throw std::runtime_error("Device does not support present queue.");
    }

    void VulkanManager::CreateDevice()
    {
        // create device queue create infos
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        float queue_priority = 1.0f;
        for (auto index : using_queue_family_indices_)
        {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = index;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(queue_create_info);
        }

        std::vector<const char*> extensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        // create logical device
        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        create_info.enabledLayerCount = 0;
        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        if (vkCreateDevice(physical_device_, &create_info, nullptr, &device_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create logical device.");
    }

    void VulkanManager::CreateSwapchain(uint32_t width, uint32_t height)
    {
        // get capabilities
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &capabilities);

        // get supported formats
        uint32_t format_count;
        std::vector<VkSurfaceFormatKHR> formats;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, nullptr);
        formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, formats.data());

        // create info
        VkSwapchainCreateInfoKHR swapchain_create_info{};
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.surface = surface_;
        swapchain_create_info.minImageCount = capabilities.minImageCount;
        swapchain_create_info.imageFormat = formats[0].format;
        swapchain_create_info.imageColorSpace = formats[0].colorSpace;
        swapchain_create_info.imageExtent =
        {
            std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, width)),
            std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, height))
        };
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        if (using_queue_family_indices_.size() > 1)
        {
            std::vector<uint32_t> indices(using_queue_family_indices_.begin(), using_queue_family_indices_.end());
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_create_info.queueFamilyIndexCount = static_cast<uint32_t>(using_queue_family_indices_.size());
            swapchain_create_info.pQueueFamilyIndices = indices.data();
        }
        else swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

        swapchain_create_info.preTransform = capabilities.currentTransform;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        swapchain_create_info.clipped = VK_TRUE;
        swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

        // create swapchain
        if (vkCreateSwapchainKHR(device_, &swapchain_create_info, nullptr, &swapchain_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create swapchain.");

        // get swapchain images
        uint32_t image_count = 0;
        vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, nullptr);
        swapchain_images_.resize(image_count);
        vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, swapchain_images_.data());

        // create image views for swapchain images
        swapchain_image_views_.resize(image_count);
        for (int i = 0; i < image_count; i++)
        {
            VkImageViewCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            create_info.image = swapchain_images_[i];
            create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            create_info.format = swapchain_create_info.imageFormat;
            create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            create_info.subresourceRange.baseMipLevel = 0;
            create_info.subresourceRange.levelCount = 1;
            create_info.subresourceRange.baseArrayLayer = 0;
            create_info.subresourceRange.layerCount = 1;
            if (vkCreateImageView(device_, &create_info, nullptr, &swapchain_image_views_[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create swapchain image views.");
        }
    }
}
