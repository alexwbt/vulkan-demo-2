#pragma once

namespace vk
{
    class VulkanManager
    {
    private:
        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        VkSurfaceKHR surface_;
        VkDevice device_;

        std::vector<VkQueueFamilyProperties> queue_families_;

    public:
        VulkanManager(GLFWwindow* window);
        ~VulkanManager();

    private:
        void CreateInstance();
        void GetPhysicalDeviceAndQueuesFamilies();
        void CreateSurface(GLFWwindow* window);
        void CreateDevice();
    };
}
