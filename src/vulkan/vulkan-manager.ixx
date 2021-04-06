module;
#include "vulkan.h"

export module Vulkan:Manager;
import :Instance;
import :Swapchain;
import :Device;

namespace vk
{
    export class VulkanManager
    {
    private:
        Instance instance_;
        Swapchain swapchain_;
        Device device_;

    public:
        bool Init(GLFWwindow* window)
        {
            try
            {
                instance_.Create(
                    {
#ifdef _DEBUG
                        "VK_LAYER_KHRONOS_validation"
#endif
                    }
                );
                swapchain_.Create(instance_.Get(), window);
                device_.Create(instance_.Get(), swapchain_.GetSurface());
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
            device_.Destroy();
            swapchain_.Destroy(instance_.Get());
            instance_.Destroy();
        }
    };
}
