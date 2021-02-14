#include "pch.h"

namespace vk
{
    VulkanManager::VulkanManager(GLFWwindow* window, uint32_t width, uint32_t height)
    {
        CreateInstance();
        CreateSurface(window);
        GetPhysicalDeviceAndQueuesFamilies();
        CreateDevice();
        CreateSwapchain(width, height);
        CreateRenderPass();
        CreateGraphicsPipeline();
        CreateFramebuffers();
        CreateCommandPool();
        CreateCommandBuffers();
    }

    VulkanManager::~VulkanManager()
    {
        vkDestroyCommandPool(device_, command_pool_, nullptr);
        for (auto framebuffer : swapchain_framebuffers_)
            vkDestroyFramebuffer(device_, framebuffer, nullptr);
        vkDestroyPipeline(device_, graphics_pipeline_, nullptr);
        vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
        vkDestroyRenderPass(device_, render_pass_, nullptr);
        for (auto image_view : swapchain_image_views_)
            vkDestroyImageView(device_, image_view, nullptr);
        vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        vkDestroyDevice(device_, nullptr);
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        vkDestroyInstance(instance_, nullptr);
    }

    void VulkanManager::CreateInstance()
    {
        // create app info
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Vulkan Demo 2";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;

        // get glfw extensions
        uint32_t glfw_extension_count = 0;
        const char** glfw_extensions;
        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        // create info
        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = glfw_extension_count;
        create_info.ppEnabledExtensionNames = glfw_extensions;
#ifdef _DEBUG
        std::vector<const char*> validation_layers =
        {
            "VK_LAYER_KHRONOS_validation"
        };
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
#else
        create_info.enabledLayerCount = 0;
#endif

        // create instance
        if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create instance.");
    }

    void VulkanManager::CreateSurface(GLFWwindow* window)
    {
        if (glfwCreateWindowSurface(instance_, window, nullptr, &surface_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create window surface.");
    }

    void VulkanManager::GetPhysicalDeviceAndQueuesFamilies()
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);

        std::cout << "Graphics Card Count: " << device_count << std::endl;

        if (device_count == 0)
            throw std::runtime_error("Failed to get physical device.");

        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());

        // get first graphics card
        physical_device_ = devices[0];
        if (physical_device_ == VK_NULL_HANDLE)
            throw std::runtime_error("Failed to get physical device.");

        // get queues families
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queue_family_count, nullptr);

        queue_families_.resize(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queue_family_count, queue_families_.data());

        // get graphics queue family index
        bool found = false;
        for (int i = 0; i < queue_families_.size(); i++)
        {
            if (queue_families_[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                // graphics queue family index
                using_queue_family_indices_.insert(i);
                found = true;
                break;
            }
        }
        if (!found) throw std::runtime_error("Device does not support graphics queue.");

        // get present queue family index
        found = false;
        for (int i = 0; i < queue_families_.size(); i++)
        {
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, i, surface_, &present_support);

            if (present_support)
            {
                // present queue family index
                using_queue_family_indices_.insert(i);
                found = true;
                break;
            }
        }
        if (!found) throw std::runtime_error("Device does not support present queue.");
    }

    void VulkanManager::CreateDevice()
    {
        // create device queue create infos
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        float queue_priority = 1.0f;
        for (auto index : using_queue_family_indices_)
        {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = index;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(queue_create_info);
        }

        std::vector<const char*> extensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        // create logical device
        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        create_info.enabledLayerCount = 0;
        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        if (vkCreateDevice(physical_device_, &create_info, nullptr, &device_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create logical device.");
    }

    void VulkanManager::CreateSwapchain(uint32_t width, uint32_t height)
    {
        // get capabilities
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &capabilities);

        // get supported formats
        uint32_t format_count;
        std::vector<VkSurfaceFormatKHR> formats;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, nullptr);
        formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, formats.data());
        swapchain_format_ = formats[0];

        swapchain_extent_ = {
            std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, width)),
            std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, height))
        };

        uint32_t image_count = std::max(3, (int)capabilities.minImageCount);
        if (capabilities.maxImageCount > 0 && capabilities.maxImageCount < image_count)
            image_count = capabilities.maxImageCount;

        // create info
        VkSwapchainCreateInfoKHR swapchain_create_info{};
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.surface = surface_;
        swapchain_create_info.minImageCount = image_count;
        swapchain_create_info.imageFormat = swapchain_format_.format;
        swapchain_create_info.imageColorSpace = swapchain_format_.colorSpace;
        swapchain_create_info.imageExtent = swapchain_extent_;
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        if (using_queue_family_indices_.size() > 1)
        {
            std::vector<uint32_t> indices(using_queue_family_indices_.begin(), using_queue_family_indices_.end());
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_create_info.queueFamilyIndexCount = static_cast<uint32_t>(using_queue_family_indices_.size());
            swapchain_create_info.pQueueFamilyIndices = indices.data();
        }
        else swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

        swapchain_create_info.preTransform = capabilities.currentTransform;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        swapchain_create_info.clipped = VK_TRUE;
        swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

        // create swapchain
        if (vkCreateSwapchainKHR(device_, &swapchain_create_info, nullptr, &swapchain_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create swapchain.");

        // get swapchain images
        uint32_t image_count = 0;
        vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, nullptr);
        swapchain_images_.resize(image_count);
        vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, swapchain_images_.data());

        // create image views for swapchain images
        swapchain_image_views_.resize(image_count);
        for (int i = 0; i < image_count; i++)
        {
            VkImageViewCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            create_info.image = swapchain_images_[i];
            create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            create_info.format = swapchain_format_.format;
            create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            create_info.subresourceRange.baseMipLevel = 0;
            create_info.subresourceRange.levelCount = 1;
            create_info.subresourceRange.baseArrayLayer = 0;
            create_info.subresourceRange.layerCount = 1;
            if (vkCreateImageView(device_, &create_info, nullptr, &swapchain_image_views_[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create swapchain image views.");
        }
    }

    void VulkanManager::CreateRenderPass()
    {
        VkAttachmentDescription color_attachment{};
        color_attachment.format = swapchain_format_.format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_ref{};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;

        VkRenderPassCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        create_info.attachmentCount = 1;
        create_info.pAttachments = &color_attachment;
        create_info.subpassCount = 1;
        create_info.pSubpasses = &subpass;

        if (vkCreateRenderPass(device_, &create_info, nullptr, &render_pass_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create render pass.");
    }

    VkShaderModule VulkanManager::CreateShaderModule(const std::string& filename)
    {
        auto code = util::ReadFile(filename);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shader_module;
        if (vkCreateShaderModule(device_, &createInfo, nullptr, &shader_module) != VK_SUCCESS)
            throw std::runtime_error("Failed to create shader module.");

        return shader_module;
    }

    void VulkanManager::CreateGraphicsPipeline()
    {
        VkShaderModule vertex_shader = CreateShaderModule("shaders/vert.spv");
        VkShaderModule fragment_shader = CreateShaderModule("shaders/frag.spv");

        VkPipelineShaderStageCreateInfo vertex_shader_stage_info{};
        vertex_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertex_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertex_shader_stage_info.module = vertex_shader;
        vertex_shader_stage_info.pName = "main";

        VkPipelineShaderStageCreateInfo fragment_shader_stage_info{};
        fragment_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragment_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragment_shader_stage_info.module = fragment_shader;
        fragment_shader_stage_info.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertex_shader_stage_info, fragment_shader_stage_info };

        VkPipelineVertexInputStateCreateInfo vertex_info_info{};
        vertex_info_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_info_info.vertexBindingDescriptionCount = 0;
        vertex_info_info.vertexAttributeDescriptionCount = 0;

        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapchain_extent_.width;
        viewport.height = (float)swapchain_extent_.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapchain_extent_;

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.scissorCount = 1;
        viewport_state.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo color_blending{};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY;
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &color_blend_attachment;
        color_blending.blendConstants[0] = 0.0f;
        color_blending.blendConstants[1] = 0.0f;
        color_blending.blendConstants[2] = 0.0f;
        color_blending.blendConstants[3] = 0.0f;

        // create pipeline layout
        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 0;
        pipeline_layout_info.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(device_, &pipeline_layout_info, nullptr, &pipeline_layout_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline layout.");

        // create pipline
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertex_info_info;
        pipelineInfo.pInputAssemblyState = &input_assembly;
        pipelineInfo.pViewportState = &viewport_state;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &color_blending;
        pipelineInfo.layout = pipeline_layout_;
        pipelineInfo.renderPass = render_pass_;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphics_pipeline_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create graphics pipline.");

        vkDestroyShaderModule(device_, fragment_shader, nullptr);
        vkDestroyShaderModule(device_, vertex_shader, nullptr);
    }

    void VulkanManager::CreateFramebuffers()
    {
        swapchain_framebuffers_.resize(swapchain_image_views_.size());

        for (int i = 0; i < swapchain_image_views_.size(); i++)
        {
            VkImageView attachments[] = { swapchain_image_views_[i] };

            VkFramebufferCreateInfo frame_buffer_info{};
            frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frame_buffer_info.renderPass = render_pass_;
            frame_buffer_info.attachmentCount = 1;
            frame_buffer_info.pAttachments = attachments;
            frame_buffer_info.width = swapchain_extent_.width;
            frame_buffer_info.height = swapchain_extent_.height;
            frame_buffer_info.layers = 1;

            if (vkCreateFramebuffer(device_, &frame_buffer_info, nullptr, &swapchain_framebuffers_[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create framebuffer.");
        }
    }

    void VulkanManager::CreateCommandPool()
    {
        VkCommandPoolCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create_info.queueFamilyIndex = *using_queue_family_indices_.begin();
        create_info.flags = 0;

        if (vkCreateCommandPool(device_, &create_info, nullptr, &command_pool_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create command pool.");
    }

    void VulkanManager::CreateCommandBuffers()
    {
        command_buffers_.resize(swapchain_framebuffers_.size());

        VkCommandBufferAllocateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.commandPool = command_pool_;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandBufferCount = (uint32_t)command_buffers_.size();

        if (vkAllocateCommandBuffers(device_, &info, command_buffers_.data()) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate command buffers.");

        // record commands for each swapchain framebuffer
        for (size_t i = 0; i < command_buffers_.size(); i++)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(command_buffers_[i], &beginInfo) != VK_SUCCESS)
                throw std::runtime_error("Failed to record command buffer.");

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = render_pass_;
            renderPassInfo.framebuffer = swapchain_framebuffers_[i];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = swapchain_extent_;

            VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(command_buffers_[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(command_buffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);

            vkCmdDraw(command_buffers_[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(command_buffers_[i]);

            if (vkEndCommandBuffer(command_buffers_[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to record command buffer.");
        }
    }
}
