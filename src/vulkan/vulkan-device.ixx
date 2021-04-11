module;
#include "vulkan.h"

export module Vulkan:Device;
import :Swapchain;

namespace vk
{
    const std::vector<const char*> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    export class Device
    {
    private:
        struct QueueFamilyIndices
        {
            int graphics = -1;
            int presentation = -1;
            bool Valid()
            {
                return graphics >= 0
                    && presentation >= 0;
            }
        };

    private:
        bool created_ = false;

        VkPhysicalDevice physical_;
        VkDevice logical_;

        QueueFamilyIndices queue_family_indices_;

        VkQueue graphics_queue_;
        VkQueue presentation_queue_;

        SwapchainDetail swapchain_detail_;

    private:
        auto GetQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface)
        {
            uint32_t count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
            std::vector<VkQueueFamilyProperties> families(count);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

            QueueFamilyIndices queue_family_indices;
            for (uint32_t family_index = 0; family_index < count; family_index++)
            {
                if (families[family_index].queueCount <= 0)
                    continue;

                if (queue_family_indices.graphics < 0 && families[family_index].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    queue_family_indices.graphics = family_index;

                if (queue_family_indices.presentation < 0)
                {
                    VkBool32 supports_presentation = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(device, family_index, surface, &supports_presentation);
                    if (supports_presentation)
                        queue_family_indices.presentation = family_index;
                }
            }

            return queue_family_indices;
        }

        bool CheckExtensionSupport(VkPhysicalDevice device)
        {
            uint32_t count = 0;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
            std::vector<VkExtensionProperties> supported_extensions(count);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &count, supported_extensions.data());

            for (const char* required_extension : device_extensions)
            {
                bool supported = false;
                for (const auto& supported_extension : supported_extensions)
                {
                    if (strncmp(required_extension, supported_extension.extensionName, VK_MAX_EXTENSION_NAME_SIZE) == 0)
                    {
                        supported = true;
                        break;
                    }
                }

                if (!supported)
                    return false;
            }
            return true;
        }


        auto GetSwapchainDetail(VkPhysicalDevice device, VkSurfaceKHR surface)
        {
            SwapchainDetail detail{};

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &detail.surface_capabilities);

            uint32_t format_count = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
            if (format_count > 0)
            {
                detail.surface_formats.resize(format_count);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, detail.surface_formats.data());
            }

            uint32_t present_mode_count = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
            if (present_mode_count > 0)
            {
                detail.present_modes.resize(present_mode_count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, detail.present_modes.data());
            }

            return detail;
        }

        bool UseDeviceIfSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
        {
            /*VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(device, &properties);

            VkPhysicalDeviceFeatures features{};
            vkGetPhysicalDeviceFeatures(device, &features);*/

            auto indices = GetQueueFamilyIndices(device, surface);
            if (!indices.Valid())
                return false;

            if (!CheckExtensionSupport(device))
                return false;

            auto swapchain_detail = GetSwapchainDetail(device, surface);
            if (!swapchain_detail.Valid())
                return false;

            physical_ = device;
            queue_family_indices_ = indices;
            swapchain_detail_ = swapchain_detail;
            return true;
        }

        void GetPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
        {
            uint32_t device_count = 0;
            vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
            std::vector<VkPhysicalDevice> devices(device_count);
            vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

            bool found = false;
            for (const auto& device : devices)
            {
                if (UseDeviceIfSuitable(device, surface))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                throw std::runtime_error("Failed to get physical device.");
        }

        void CreateLogicalDevice()
        {
            std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
            std::set<int> create_queue_family_indices = {
                queue_family_indices_.graphics,
                queue_family_indices_.presentation
            };
            float priority = 1.0f;

            for (int family_index : create_queue_family_indices)
            {
                VkDeviceQueueCreateInfo queue_create_info{};
                queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_info.queueFamilyIndex = family_index;
                queue_create_info.queueCount = 1;
                queue_create_info.pQueuePriorities = &priority;

                queue_create_infos.push_back(queue_create_info);
            }

            VkDeviceCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
            create_info.pQueueCreateInfos = queue_create_infos.data();
            create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
            create_info.ppEnabledExtensionNames = device_extensions.data();

            //create_info.pEnabledFeatures = nullptr;

            if (vkCreateDevice(physical_, &create_info, nullptr, &logical_) != VK_SUCCESS)
                throw std::runtime_error("Failed to create logical device.");

            vkGetDeviceQueue(logical_, queue_family_indices_.graphics, 0, &graphics_queue_);
            vkGetDeviceQueue(logical_, queue_family_indices_.presentation, 0, &presentation_queue_);
        }

    public:
        void Create(VkInstance instance, VkSurfaceKHR surface)
        {
#ifdef _DEBUG
            if (created_) throw std::runtime_error("Tried to create device before destruction.");
            if (!instance) throw std::runtime_error("Tried to create device with invalid instance.");
#endif
            GetPhysicalDevice(instance, surface);
            CreateLogicalDevice();
            created_ = true;
        }

        void Destroy()
        {
#ifdef _DEBUG
            if (!created_) throw std::runtime_error("Tried to destroy device before creation.");
#endif

            created_ = false;
            vkDestroyDevice(logical_, nullptr);
        }

        VkDevice Get()
        {
#ifdef _DEBUG
            if (!created_) throw std::runtime_error("Tried to get device before creation.");
#endif
            return logical_;
        }

        SwapchainDetail GetSwapchainDetail()
        {
#ifdef _DEBUG
            if (!created_) throw std::runtime_error("Tried to get swapchain before device creation.");
#endif
            return swapchain_detail_;
        }
    };
}
