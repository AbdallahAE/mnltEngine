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
            Window(int w, int h, std::string name);
            ~Window();

            Window(const Window &) = delete;
            Window &operator=(const Window &) = delete;

            bool shouldClose() { return glfwWindowShouldClose(window); }
            bool wasWindowResized() { return framebufferResized; }
            void resetWindowResizedFlag() { framebufferResized = false; }

            VkExtent2D getExtent() {return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};}

            void createWindowSurface(VkInstance instace, VkSurfaceKHR *surface);

        private:
            int width;
            int height;
            bool framebufferResized = false;

            std::string windowName;
            GLFWwindow *window;
            
            static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
            void initWindow();
    };
}