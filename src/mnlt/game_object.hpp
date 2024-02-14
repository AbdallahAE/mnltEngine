#pragma once

#include "model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace mnlt
{
    struct PointLightComponent
    {
        float lightIntensity = 1.0f;
    };
    struct RigidBodyComponent 
    {
        glm::vec3 velocity;
        float mass{1.0f};
    };
    struct TransformComponent
    {
        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation;

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 mat4() 
        {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);
            return glm::mat4{
                {
                    scale.x * (c1 * c3 + s1 * s2 * s3),
                    scale.x * (c2 * s3),
                    scale.x * (c1 * s2 * s3 - c3 * s1),
                    0.0f,
                },
                {
                    scale.y * (c3 * s1 * s2 - c1 * s3),
                    scale.y * (c2 * c3),
                    scale.y * (c1 * c3 * s2 + s1 * s3),
                    0.0f,
                },
                {
                    scale.z * (c2 * s1),
                    scale.z * (-s2),
                    scale.z * (c1 * c2),
                    0.0f,
                },
                {translation.x, translation.y, translation.z, 1.0f}};
        };

        glm::mat3 normalMatrix()
        {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);
            const glm::vec3 invScale = 1.0f / scale;

            return glm::mat3
            {
            {
                    invScale.x * (c1 * c3 + s1 * s2 * s3),
                    invScale.x * (c2 * s3),
                    invScale.x * (c1 * s2 * s3 - c3 * s1),
                },
            {
                    invScale.y * (c3 * s1 * s2 - c1 * s3),
                    invScale.y * (c2 * c3),
                    invScale.y * (c1 * c3 * s2 + s1 * s3),
                },
            {
                    invScale.z * (c2 * s1),
                    invScale.z * (-s2),
                    invScale.z * (c1 * c2),
                },
            };
        };
    };
    struct UIComponent
    {
        bool showPropertyWindow = false;
    };

    class GameObject
    {
        public:
            using id_t = unsigned int;
            using Map = std::unordered_map<id_t, GameObject>;

            static GameObject makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f))
            {
                GameObject gameObj = GameObject::createGameObject();
                gameObj.color = color;
                gameObj.transform.scale.x = radius;
                gameObj.pointLight = std::make_unique<PointLightComponent>();
                gameObj.pointLight->lightIntensity = intensity;
                return gameObj;
            }

            static GameObject createGameObject()
            {
                static id_t currentId = 0;
                return GameObject(currentId++);
            }
        
            GameObject(const GameObject &) = delete;
            GameObject &operator=(const GameObject &) = delete;
            GameObject(GameObject &&) = default;
            GameObject &operator=(GameObject &&) = default;

            id_t getId() { return id; }

            std::shared_ptr<Model> model {};
            glm::vec3 color{1.f,1.f,1.f};
            TransformComponent transform{};
            RigidBodyComponent rigidBody{};
            UIComponent ui;
            std::string name;
            std::unique_ptr<PointLightComponent> pointLight = nullptr;

        private:
            GameObject(id_t objId) : id{objId}
            {
                name = "GameObject" + std::to_string(id);
            }
            id_t id;

    };
}