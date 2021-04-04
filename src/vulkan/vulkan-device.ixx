module;
#include "vulkan.h"

export module Vulkan:Device;

namespace vk
{
    export using QueueFamilyIndices = std::map<VkQueueFlagBits, int>;

    export class Device
    {
    private:
        bool created_ = false;

        VkPhysicalDevice physical_;
        VkDevice logical_;

        std::unique_ptr<QueueFamilyIndices> queue_family_indices_;

        VkQueue graphics_queue_;

    private:
        auto GetQueueFamilyIndices(VkPhysicalDevice device, const std::vector<VkQueueFlagBits>& required_queue_families)
        {
            uint32_t count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
            std::vector<VkQueueFamilyProperties> families(count);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

            auto queue_family_indices = std::make_unique<QueueFamilyIndices>();
            for (uint32_t i = 0; i < count; i++)
                if (families[i].queueCount > 0)
                    for (const auto& required_family : required_queue_families) // check
                        if (families[i].queueFlags & required_family)
                            queue_family_indices->insert({ required_family, i });

            return queue_family_indices;
        }

        bool UseDeviceIfSuitable(VkPhysicalDevice device, const std::vector<VkQueueFlagBits>& required_queue_families)
        {
            /*VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(device, &properties);

            VkPhysicalDeviceFeatures features{};
            vkGetPhysicalDeviceFeatures(device, &features);*/

            auto indices = GetQueueFamilyIndices(device, required_queue_families);
            for (const auto& [key, value] : *indices)
                if (value == -1)
                    return false;

            physical_ = device;
            queue_family_indices_ = std::move(indices);
            return true;
        }

        void GetPhysicalDevice(VkInstance instance, const std::vector<VkQueueFlagBits>& required_queue_families)
        {
            uint32_t device_count = 0;
            vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

            if (!device_count)
                throw std::runtime_error("Failed to find physical device.");

            std::vector<VkPhysicalDevice> devices(device_count);
            vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

            for (const auto& device : devices)
                if (UseDeviceIfSuitable(device, required_queue_families))
                    break;
        }

        void CreateLogicalDevice()
        {
            float graphics_queue_priorities[1] = { 1.0f };
            VkDeviceQueueCreateInfo graphics_queue_create_info{};
            graphics_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            graphics_queue_create_info.queueFamilyIndex = queue_family_indices_->at(VK_QUEUE_GRAPHICS_BIT);
            graphics_queue_create_info.queueCount = 1;
            graphics_queue_create_info.pQueuePriorities = graphics_queue_priorities;

            VkDeviceQueueCreateInfo queue_create_infos[1] = { graphics_queue_create_info };

            VkDeviceCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            create_info.queueCreateInfoCount = 1;
            create_info.pQueueCreateInfos = queue_create_infos;
            create_info.enabledExtensionCount = 0;
            create_info.ppEnabledExtensionNames = nullptr;
            //create_info.pEnabledFeatures = nullptr;

            if (vkCreateDevice(physical_, &create_info, nullptr, &logical_) != VK_SUCCESS)
                throw std::runtime_error("Failed to create logical device.");

            vkGetDeviceQueue(logical_, queue_family_indices_->at(VK_QUEUE_GRAPHICS_BIT), 0, &graphics_queue_);
        }

    public:
        void Create(VkInstance instance, const std::vector<VkQueueFlagBits>& required_queue_families)
        {
            GetPhysicalDevice(instance, required_queue_families);
            CreateLogicalDevice();
            created_ = true;
        }

        void Destroy()
        {
            created_ = false;
            vkDestroyDevice(logical_, nullptr);
        }
    };
}
