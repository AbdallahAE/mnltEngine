#pragma once

#include "device.hpp"
#include "frame_info.hpp"
#include "window.hpp"

// libs
#include "../../libs/imgui/imgui.h"

// This whole class is only necessary right now because it needs to manage the descriptor pool
// because we haven't set one up anywhere else in the application, and we manage the
// example state, otherwise all the functions could just be static helper functions if you prefered
namespace mnlt
{
    static void check_vk_result(VkResult err) 
    {
        if (err == 0) return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0) abort();
    }

    class UI
    {
        public:
            UI(Window &window, Device &device, VkRenderPass renderPass, uint32_t imageCount, VkDescriptorPool descriptorPool);
            ~UI();

            void newFrame();

            void render(VkCommandBuffer commandBuffer);

            // Example state
            bool show_debug_window = false;
            void runExample(GameObject::Map* gameObjects);

        private:
            void showGameObjectWindow(GameObject *gameObject);
            Device &device;
    };
}