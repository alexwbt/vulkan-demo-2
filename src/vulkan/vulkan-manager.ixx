module;
#include "vulkan.h"

export module Vulkan:Manager;
import :Instance;
import :Surface;
import :Device;
import :Swapchain;

namespace vk
{
    export class VulkanManager
    {
    private:
        Instance instance_;
        Surface surface_;
        Device device_;
        Swapchain swapchain_;

    public:
        bool Init(GLFWwindow* window)
        {
            try
            {
                instance_.Create(
#ifdef _DEBUG
                    { "VK_LAYER_KHRONOS_validation" }
#endif
                );
                surface_.Create(instance_.Get(), window);
                device_.Create(instance_.Get(), surface_.Get());
                swapchain_.Create(device_.GetSwapchainDetail());
            }
            catch (const std::runtime_error& error)
            {
                std::cout << "Error: " << error.what() << std::endl;
                return false;
            }
            return true;
        }

        void Clean()
        {
            swapchain_.Destroy();
            device_.Destroy();
            surface_.Destroy(instance_.Get());
            instance_.Destroy();
        }
    };
}
