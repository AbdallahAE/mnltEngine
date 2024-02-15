#pragma once

#include "camera.hpp"
#include "game_object.hpp"
#include "time.hpp"

// lib
#include <vulkan/vulkan.h>

namespace mnlt 
{
    #define MAX_LIGHTS 10

    struct PointLight 
    {
        glm::vec4 position{};  // ignore w
        glm::vec4 color{};     // w is intensity
    };
    struct DirectionalLight 
    {
        glm::vec4 position{};   // ignore w
        glm::vec4 color{};  // w is intensity
    };

    struct GlobalUbo 
    {
        alignas(16) glm::mat4 projection{1.f};
        alignas(16) glm::mat4 view{1.f};
        alignas(16) glm::mat4 inverseView{1.f};
        alignas(16) DirectionalLight directionalLight;
        alignas(16) glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f};  // w is intensity
        alignas(16) PointLight pointLights[MAX_LIGHTS];
        alignas(16) int numLights;
    };

    struct FrameInfo 
    {
        int frameIndex;
        Time &time;
        VkCommandBuffer commandBuffer;
        Camera &camera;
        VkDescriptorSet globalDescriptorSet;
        GameObject::Map &gameObjects;
    };
}