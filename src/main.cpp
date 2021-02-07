#include "pch.h"

int main()
{
    try
    {
        // init window
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

        vk::VulkanManager vk_manager;

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
