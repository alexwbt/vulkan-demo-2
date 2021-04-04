#include <GLFW/glfw3.h>

import Vulkan;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    vk::VulkanManager vk_manager{};
    vk_manager.Init(window);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    vk_manager.Terminate();

    glfwDestroyWindow(window);
    glfwTerminate();
}
