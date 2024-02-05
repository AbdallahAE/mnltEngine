#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <string>
#include <cstdint>


namespace mnlt
{
    class Window
    {
        public:
            Window(int width, int height, std::string name);
            ~Window();

            Window(const Window &) = delete;
            Window &operator=(const Window &) = delete;

            bool shouldClose() { return glfwWindowShouldClose(window); }
            VkExtent2D getExtent() {return {static_cast<uint32_t>(WIDTH), static_cast<uint32_t>(HEIGHT)};}

            void createWindowSurface(VkInstance instace, VkSurfaceKHR *surface);

        private:
            GLFWwindow *window;
            const int HEIGHT;
            const int WIDTH;
            std::string windowName;
            
            void initWindow();
    };
}