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
        VkSwapchainKHR swapchain_;
        std::vector<VkImage> swapchain_images_;
        std::vector<VkImageView> swapchain_image_views_;

        std::vector<VkQueueFamilyProperties> queue_families_;
        std::set<uint32_t> using_queue_family_indices_;

    public:
        VulkanManager(GLFWwindow* window, uint32_t width, uint32_t height);
        ~VulkanManager();

    private:
        void CreateInstance();
        void CreateSurface(GLFWwindow* window);
        void GetPhysicalDeviceAndQueuesFamilies();
        void CreateDevice();
        void CreateSwapchain(uint32_t width, uint32_t height);

    public:
        // getters
        VkDevice device() const { return device_; }
    };
}
