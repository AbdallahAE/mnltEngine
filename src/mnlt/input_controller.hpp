#pragma once

#include "game_object.hpp"
#include <GLFW/glfw3.h>

namespace mnlt
{
    class CameraController 
    {
        public:
            struct KeyMappings 
            {
                int moveLeft = GLFW_KEY_A;
                int moveRight = GLFW_KEY_D;
                int moveForward = GLFW_KEY_W;
                int moveBackward = GLFW_KEY_S;
                int moveUp = GLFW_KEY_E;
                int moveDown = GLFW_KEY_Q;
                int look = GLFW_MOUSE_BUTTON_RIGHT;
            };

            void moveInPlaneXZ(GLFWwindow* window, float dt, GameObject& gameObject);
            void mouseLook(GLFWwindow* window, int button, int action, int mods);

            KeyMappings keys{};
            float moveSpeed{3.f};
            float lookSpeed{2.0f};
    };
}