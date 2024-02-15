#include "input_controller.hpp"

#include <GLFW/glfw3.h>
#include <limits>

namespace mnlt
{
    double prevMouseY = 0.0;
    double prevMouseX = 0.0;
    void CameraController::moveInPlaneXZ(GLFWwindow* window, double pureDeltaTime, GameObject& gameObject) 
    {
        glm::vec3 rotate{0};
        
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        float deltaY = static_cast<float>(mouseY - prevMouseY);
        float deltaX = static_cast<float>(mouseX - prevMouseX);
        if(glfwGetMouseButton(window, keys.look) == GLFW_PRESS)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            if(deltaY != 0)
                rotate.x += deltaY;
            if(deltaX != 0)
                rotate.y -= deltaX;
        }
        if(glfwGetMouseButton(window, keys.look) == GLFW_RELEASE)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        prevMouseY = mouseY;
        prevMouseX = mouseX;
        float mouseSpeed = sqrt(deltaX * deltaX + deltaY * deltaY) / static_cast<float>(pureDeltaTime) * 0.001 * lookSpeed;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.rotation += mouseSpeed * static_cast<float>(pureDeltaTime) * glm::normalize(rotate);
        }

        // limit pitch values between about +/- 85ish degrees
        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

        float yaw = gameObject.transform.rotation.y;
        float pitch = -gameObject.transform.rotation.x;
        const glm::vec3 forwardDir
        {
            sin(yaw) * cos(pitch), 
            sin(pitch),
            cos(yaw) * cos(pitch)
            };
        const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
        const glm::vec3 upDir{0.f, -1.f, 0.f};

        glm::vec3 moveDir{0.f};
        if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
        if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
        if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
        if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
        if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
        if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) 
        {
            gameObject.transform.translation += moveSpeed * static_cast<float>(pureDeltaTime) * glm::normalize(moveDir);
        }
    }
}