#include "camera.hpp"

// std
#include <GLFW/glfw3.h>
#include <cassert>
#include <glm/gtc/constants.hpp>
#include <limits>

namespace mnlt 
{
    void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
        projectionMatrix = glm::mat4{1.0f};
        projectionMatrix[0][0] = 2.f / (right - left);
        projectionMatrix[1][1] = 2.f / (bottom - top);
        projectionMatrix[2][2] = 1.f / (far - near);
        projectionMatrix[3][0] = -(right + left) / (right - left);
        projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
        projectionMatrix[3][2] = -near / (far - near);
        this->near = near;
        this->far = far;
    }

    void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far) 
    {
        assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
        const float tanHalfFovy = tan(fovy / 2.f);
        projectionMatrix = glm::mat4{0.0f};
        projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
        projectionMatrix[1][1] = 1.f / (tanHalfFovy);
        projectionMatrix[2][2] = far / (far - near);
        projectionMatrix[2][3] = 1.f;
        projectionMatrix[3][2] = -(far * near) / (far - near);
        this->near = near;
        this->far = far;
    }

    void Camera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) 
    {
        const glm::vec3 w{glm::normalize(direction)};
        const glm::vec3 u{glm::normalize(glm::cross(w, up))};
        const glm::vec3 v{glm::cross(w, u)};

        viewMatrix = glm::mat4{1.f};
        viewMatrix[0][0] = u.x;
        viewMatrix[1][0] = u.y;
        viewMatrix[2][0] = u.z;
        viewMatrix[0][1] = v.x;
        viewMatrix[1][1] = v.y;
        viewMatrix[2][1] = v.z;
        viewMatrix[0][2] = w.x;
        viewMatrix[1][2] = w.y;
        viewMatrix[2][2] = w.z;
        viewMatrix[3][0] = -glm::dot(u, position);
        viewMatrix[3][1] = -glm::dot(v, position);
        viewMatrix[3][2] = -glm::dot(w, position);

        inverseViewMatrix = glm::mat4{1.f};
        inverseViewMatrix[0][0] = u.x;
        inverseViewMatrix[0][1] = u.y;
        inverseViewMatrix[0][2] = u.z;
        inverseViewMatrix[1][0] = v.x;
        inverseViewMatrix[1][1] = v.y;
        inverseViewMatrix[1][2] = v.z;
        inverseViewMatrix[2][0] = w.x;
        inverseViewMatrix[2][1] = w.y;
        inverseViewMatrix[2][2] = w.z;
        inverseViewMatrix[3][0] = position.x;
        inverseViewMatrix[3][1] = position.y;
        inverseViewMatrix[3][2] = position.z;
    }

    void Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) 
    {
        setViewDirection(position, target - position, up);
    }

    void Camera::updateView() 
    {
        const float c3 = glm::cos(viewerObject.rotation.z);
        const float s3 = glm::sin(viewerObject.rotation.z);
        const float c2 = glm::cos(viewerObject.rotation.x);
        const float s2 = glm::sin(viewerObject.rotation.x);
        const float c1 = glm::cos(viewerObject.rotation.y);
        const float s1 = glm::sin(viewerObject.rotation.y);
        const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
        const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
        const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
        viewMatrix = glm::mat4{1.f};
        viewMatrix[0][0] = u.x;
        viewMatrix[1][0] = u.y;
        viewMatrix[2][0] = u.z;
        viewMatrix[0][1] = v.x;
        viewMatrix[1][1] = v.y;
        viewMatrix[2][1] = v.z;
        viewMatrix[0][2] = w.x;
        viewMatrix[1][2] = w.y;
        viewMatrix[2][2] = w.z;
        viewMatrix[3][0] = -glm::dot(u, viewerObject.translation);
        viewMatrix[3][1] = -glm::dot(v, viewerObject.translation);
        viewMatrix[3][2] = -glm::dot(w, viewerObject.translation);

        inverseViewMatrix = glm::mat4{1.f};
        inverseViewMatrix[0][0] = u.x;
        inverseViewMatrix[0][1] = u.y;
        inverseViewMatrix[0][2] = u.z;
        inverseViewMatrix[1][0] = v.x;
        inverseViewMatrix[1][1] = v.y;
        inverseViewMatrix[1][2] = v.z;
        inverseViewMatrix[2][0] = w.x;
        inverseViewMatrix[2][1] = w.y;
        inverseViewMatrix[2][2] = w.z;
        inverseViewMatrix[3][0] = viewerObject.translation.x;
        inverseViewMatrix[3][1] = viewerObject.translation.y;
        inverseViewMatrix[3][2] = viewerObject.translation.z;
    }

    void Camera::move(GLFWwindow* window, double pureDeltaTime) 
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
                rotate.x -= deltaY;
            if(deltaX != 0)
                rotate.y += deltaX;
        }
        if(glfwGetMouseButton(window, keys.look) == GLFW_RELEASE)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        prevMouseY = mouseY;
        prevMouseX = mouseX;
        float mouseSpeed = sqrt(deltaX * deltaX + deltaY * deltaY) / static_cast<float>(pureDeltaTime) * 0.001 * lookSpeed;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            viewerObject.rotation += mouseSpeed * static_cast<float>(pureDeltaTime) * glm::normalize(rotate);
        }

        // limit pitch values between about +/- 85ish degrees
        viewerObject.rotation.x = glm::clamp(viewerObject.rotation.x, -1.5f, 1.5f);
        viewerObject.rotation.y = glm::mod(viewerObject.rotation.y, glm::two_pi<float>());

        float yaw = viewerObject.rotation.y;
        float pitch = -viewerObject.rotation.x;
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
            if(glfwGetKey(window, keys.speedBoost) == GLFW_PRESS)
                viewerObject.translation += (moveSpeed + speedBoost) * static_cast<float>(pureDeltaTime) * glm::normalize(moveDir);
            else
                viewerObject.translation += moveSpeed * static_cast<float>(pureDeltaTime) * glm::normalize(moveDir);
        }
        updateView();
    }
} 
