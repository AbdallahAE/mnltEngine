#include "app.hpp"
#include <GLFW/glfw3.h>

namespace mnlt
{
    void App::run()
    {
        while (!window.shouldClose())
        {
            glfwPollEvents();
        }
    }
}