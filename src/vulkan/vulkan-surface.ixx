module;
#include "vulkan.h"

export module Vulkan:Surface;

namespace vk
{
    export class Surface
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

        VkSurfaceKHR Get()
        {
#ifdef _DEBUG
            if (!created_) throw std::runtime_error("Tried to get surface before creation.");
#endif
            return surface_;
        }
    };
}
