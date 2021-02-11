#pragma once

namespace vk
{
    class Renderer
    {
    public:
        Renderer(std::shared_ptr<VulkanManager> vulkan_manager);

    private:
        std::shared_ptr<VulkanManager> vkmgr_;

        VkShaderModule CreateShaderModule(const std::string& filename);
        void InitPipeline();
    };
}
