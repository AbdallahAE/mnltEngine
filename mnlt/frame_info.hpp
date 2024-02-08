#pragma once

#include "camera.hpp"

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
    };
}