#include "pch.h"

int main()
{
    try
    {
        constexpr uint32_t width = 800, height = 600;

        // init window
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(width, height, "Vulkan window", nullptr, nullptr);

        vk::VulkanManager vk_manager(window, width, height);
        vk::Renderer renderer(vk_manager);

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

        }


        glfwDestroyWindow(window);
        glfwTerminate();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}
