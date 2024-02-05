#include "window.hpp"

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include <stdexcept>

namespace mnlt 
{
    Window::Window(int width, int height, std::string name) : WIDTH(width), HEIGHT(height), windowName(name)
    {
        initWindow();
    }

    Window::~Window()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Window::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, windowName.c_str(), nullptr, nullptr);

    }

    void Window::createWindowSurface(VkInstance instace, VkSurfaceKHR *surface)
    {
        if(glfwCreateWindowSurface(instace, window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create window surface");
        }
    }
}