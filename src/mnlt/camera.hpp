#pragma once

#include "game_object.hpp"
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace mnlt 
{
    class Camera 
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

            void setOrthographicProjection(float left, float right, float bottom, float top, float near, float far);
            void setPerspectiveProjection(float fovy, float aspect, float near, float far);

            void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
            void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
            void updateView();
            void move(GLFWwindow* window, double pureDeltaTime);


            const glm::mat4& getProjection() const { return projectionMatrix; }
            const glm::mat4& getView() const { return viewMatrix; }
            const glm::mat4& getInverseView() const { return inverseViewMatrix; }
            const glm::vec3 getPosition() const { return glm::vec3(inverseViewMatrix[3]); }

            float getNear() {return near;}
            float getFar() {return far;}
            int gridSize{1};
            bool enableGrid{false};

            GameObject viewerObject = GameObject::createGameObject();

        private:
            glm::mat4 projectionMatrix{1.f};
            glm::mat4 viewMatrix{1.f};
            glm::mat4 inverseViewMatrix{1.f};
            KeyMappings keys{};
            float moveSpeed{3.f};
            float lookSpeed{2.0f};
            float near{0.1};
            float far{100};

            double prevMouseY = 0.0;
            double prevMouseX = 0.0;
    };
} 