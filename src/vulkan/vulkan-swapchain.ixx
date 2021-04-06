module;
#include "vulkan.h"

export module Vulkan:Swapchain;

namespace vk
{
    export struct SurfaceProperties
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
        bool Valid()
        {
            return !formats.empty()
                && !present_modes.empty();
        }
    };;

    export class Swapchain
    {
    private:
        bool created_ = false;

        VkSurfaceKHR surface_;

    public:

        void Create(VkInstance instance, GLFWwindow* window)
        {
#ifdef _DEBUG
            if (created_) throw std::runtime_error("Tried to create swapchain before destruction.");
            if (!instance) throw std::runtime_error("Tried to create swapchain with invalid instance.");
            if (!window) throw std::runtime_error("Tried to create swapchain with invalid window.");
#endif
            if (glfwCreateWindowSurface(instance, window, nullptr, &surface_) != VK_SUCCESS)
                throw std::runtime_error("Failed to create surface.");
            created_ = true;
        }

        void Destroy(VkInstance instance)
        {
#ifdef _DEBUG
            if (!created_) throw std::runtime_error("Tried to destroy swapchain before creation.");
            if (!instance) throw std::runtime_error("Tried to destroy swapchain with invalid instance.");
#endif
            vkDestroySurfaceKHR(instance, surface_, nullptr);
            created_ = false;
        }

        VkSurfaceKHR GetSurface()
        {
#ifdef _DEBUG
            if (!created_) throw std::runtime_error("Tried to get surface before creation.");
#endif
            return surface_;
        }
    };
}
