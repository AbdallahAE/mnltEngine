#pragma once

#include "device.hpp"
#include "frame_info.hpp"
#include "window.hpp"

// could make functions static helper functions
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
            UI(Window &window, Device &device);
            ~UI();

            void initialize(VkRenderPass renderPass, uint32_t imageCount, VkDescriptorPool descriptorPool);

            void newFrame();

            void render(VkCommandBuffer commandBuffer);

            bool show_debug_window = false;
            void runExample(FrameInfo frameInfo);

        private:
            void showGameObjectWindow(GameObject *gameObject);
            Device &device;
            Window &window;
    };
}