module;
#include "vulkan.h"

export module Vulkan:Instance;

namespace vk
{
    export class Instance
    {
    private:
        bool created_ = false;

        VkInstance instance_;

    private:
        void CheckExtensionSupport(uint32_t count, const char* const* names)
        {
            uint32_t supported_count = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &supported_count, nullptr);
            std::vector<VkExtensionProperties> supported_extensions(supported_count);
            vkEnumerateInstanceExtensionProperties(nullptr, &supported_count, supported_extensions.data());

            for (uint32_t i = 0; i < count; i++)
            {
                bool supported = false;
                for (const auto& supported_extension : supported_extensions)
                {
                    if (strncmp(names[i], supported_extension.extensionName, VK_MAX_EXTENSION_NAME_SIZE) == 0)
                    {
                        supported = true;
                        break;
                    }
                }

                if (!supported)
                    throw std::runtime_error("Required extensions not supported.");
            }
        }

        void GetRequiredExtensions(uint32_t& count, const char* const*& names)
        {
            // get glfw required extensions
            uint32_t glfw_extension_count = 0;
            const char** glfw_extensions;
            glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

            count = glfw_extension_count;
            names = glfw_extensions;
            CheckExtensionSupport(count, names);
        }

        void CheckLayerSupport(const std::vector<const char*>& validation_layers)
        {
            uint32_t count = 0;
            vkEnumerateInstanceLayerProperties(&count, nullptr);
            std::vector<VkLayerProperties> supported_layers(count);
            vkEnumerateInstanceLayerProperties(&count, supported_layers.data());

            for (const char* layer : validation_layers)
            {
                bool supported = true;
                for (const auto& supported_layer : supported_layers)
                {
                    if (strncmp(layer, supported_layer.layerName, VK_MAX_EXTENSION_NAME_SIZE) == 0)
                    {
                        supported = true;
                        break;
                    }
                }

                if (!supported)
                    throw std::runtime_error("Required layers not supported.");
            }
        }

    public:
        void Create(const std::vector<const char*>& validation_layers)
        {
#ifdef _DEBUG
            if (created_) throw std::runtime_error("Tried to create instance before destruction.");
#endif

            VkApplicationInfo app_info{};
            app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            app_info.apiVersion = VK_API_VERSION_1_0;

            VkInstanceCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            create_info.pApplicationInfo = &app_info;
            GetRequiredExtensions(
                create_info.enabledExtensionCount,
                create_info.ppEnabledExtensionNames);
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();
            CheckLayerSupport(validation_layers);

            if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS)
                throw std::runtime_error("Failed to create vulkan instance.");

            created_ = true;
        }

        void Destroy()
        {
#ifdef _DEBUG
            if (!created_) throw std::runtime_error("Tried to destroy instance before creation.");
#endif
            created_ = false;
            vkDestroyInstance(instance_, nullptr);
        }

        VkInstance Get()
        {
            if (!created_) throw std::runtime_error("Tried to get instance before creation.");
            return instance_;
        }
    };
}
