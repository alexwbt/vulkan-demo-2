#pragma once

namespace vk
{
    class VulkanManager
    {
    private:
        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        VkDevice device_;

        std::vector<VkQueueFamilyProperties> queue_families_;
        std::vector<float> queue_priorities_ = { 1.0f };

    public:
        VulkanManager();
        ~VulkanManager();

    private:
        void CreateInstance();
        void GetPhysicalDeviceAndQueuesFamilies();
        void CreateDevice();
    };
}
