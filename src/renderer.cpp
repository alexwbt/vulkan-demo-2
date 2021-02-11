#include "pch.h"

namespace vk
{
    Renderer::Renderer(std::shared_ptr<VulkanManager> vkmgr)
        : vkmgr_(std::move(vkmgr))
    {
        InitPipeline();
    }

    VkShaderModule Renderer::CreateShaderModule(const std::string& filename)
    {
        auto code = util::ReadFile(filename);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shader_module;
        if (vkCreateShaderModule(vkmgr_->device(), &createInfo, nullptr, &shader_module) != VK_SUCCESS)
            throw std::runtime_error("Failed to create shader module.");

        return shader_module;
    }

    void Renderer::InitPipeline()
    {
        auto vertex_shader = CreateShaderModule("shaders/vert.spv");
        auto fragment_shader = CreateShaderModule("shdares/frag.spv");

        VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info{};
        vertex_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertex_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertex_shader_stage_create_info.module = vertex_shader;
        vertex_shader_stage_create_info.pName = "main";

        VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info{};
        fragment_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragment_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragment_shader_stage_create_info.module = fragment_shader;
        fragment_shader_stage_create_info.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertex_shader_stage_create_info, fragment_shader_stage_create_info };

        vkDestroyShaderModule(vkmgr_->device(), vertex_shader, nullptr);
        vkDestroyShaderModule(vkmgr_->device(), fragment_shader, nullptr);
    }
}
