module;
#include "vulkan.h"

export module Vulkan:Manager;
import :Instance;
import :Device;

namespace vk
{
    export class VulkanManager
    {
    private:
        Instance instance_;
        Device device_;

    public:
        bool Init(GLFWwindow* window)
        {
            try
            {
                instance_.Create();
                device_.Create(instance_.Get(), { VK_QUEUE_GRAPHICS_BIT });
            }
            catch (const std::runtime_error& error)
            {
                std::cout << error.what() << std::endl;
                return false;
            }
            return true;
        }

        void Terminate()
        {
            device_.Destroy();
            instance_.Destroy();
        }
    };
}
