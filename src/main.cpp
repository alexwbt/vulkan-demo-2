#include <GLFW/glfw3.h>
#include <iostream>

import Vulkan;

int main()
{
    if (glfwInit() != GLFW_TRUE)
    {
        std::cout << "Failed to initialize glfw." << std::endl;
        return 1;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    vk::VulkanManager vk_manager{};
    if (!vk_manager.Init(window))
    {
        std::cout << "Failed to initialize vulkan." << std::endl;
        return 1;
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    vk_manager.Clean();

    glfwDestroyWindow(window);
    glfwTerminate();
}
