#pragma once

#include "camera.hpp"
#include "game_object.hpp"

// lib
#include <vulkan/vulkan.h>

namespace mnlt 
{
    struct FrameInfo 
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        Camera &camera;
        VkDescriptorSet globalDescriptorSet;
        GameObject::Map &gameObjects;
    };
}