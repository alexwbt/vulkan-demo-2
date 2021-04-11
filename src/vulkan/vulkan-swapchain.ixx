module;
#include "vulkan.h"

export module Vulkan:Swapchain;

namespace vk
{
    export struct SwapchainDetail
    {
        VkSurfaceCapabilitiesKHR surface_capabilities;
        std::vector<VkSurfaceFormatKHR> surface_formats;
        std::vector<VkPresentModeKHR> present_modes;
        bool Valid()
        {
            return !surface_formats.empty()
                && !present_modes.empty();
        }
    };

    export class Swapchain
    {
    private:
        bool created_ = false;

        VkSwapchainKHR swapchain_;

    private:
        VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
        {
            if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
                return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

            for (const auto& format : formats)
                if (format.format == VK_FORMAT_R8G8B8A8_UNORM &&
                    format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return format;

            return formats[0];
        }

        VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& modes)
        {
            for (const auto& mode : modes)
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                    return mode;
            return VK_PRESENT_MODE_FIFO_KHR;
        }

    public:
        void Create(SwapchainDetail detail)
        {
#ifdef _DEBUG
            if (created_) throw std::runtime_error("Tried to create swapchain before destruction.");
            if (!detail.Valid()) throw std::runtime_error("Tried to create swapchain with invalid detail.");
#endif

            auto format = ChooseSurfaceFormat(detail.surface_formats);
            auto present_mode = ChoosePresentMode(detail.present_modes);


            created_ = true;
        }

        void Destroy()
        {
#ifdef _DEBUG
            if (!created_) throw std::runtime_error("Tried to destroy swapchain before creation.");
#endif
            created_ = false;
        }
    };
}
