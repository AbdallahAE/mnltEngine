#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <string>


namespace mnlt
{
    class Window
    {
        public:
            Window(int width, int height, std::string name);
            ~Window();

            Window(const Window &) = delete;
            Window &operator=(const Window &) = delete;

            bool shouldClose()
            {
                return glfwWindowShouldClose(window);
            }

            void createWindowSurface(VkInstance instace, VkSurfaceKHR *surface);

        private:
            GLFWwindow *window;
            const int HEIGHT;
            const int WIDTH;
            std::string windowName;
            
            void initWindow();
    };
}