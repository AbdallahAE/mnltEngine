#pragma once

#include "window.hpp"
#include "device.hpp"
#include "pipeline.hpp"
#include "swap_chain.hpp"
#include "model.hpp"

#include <memory>

namespace mnlt
{
    class App
    {
        public:
            constexpr static int WIDTH = 800;
            constexpr static int HEIGHT = 600;

            App();
            ~App();
            App(const App &) = delete;
            App &operator=(const App &) = delete;

            void run();

        private:
            void loadModels();
            void createPipelineLayout();
            void createPipeline();
            void createCommandBuffers();
            void freeCommandBuffers();
            void drawFrame();
            void recreateSwapChain();
            void recordCommandBuffer(int imageIndex);

            Window window{WIDTH, HEIGHT, "MoonLight"};
            Device device{window};
            std::unique_ptr<SwapChain> swapChain;
            std::unique_ptr<Pipeline> pipeline;
            VkPipelineLayout pipelineLayout;
            std::vector<VkCommandBuffer> commandBuffers;
            std::unique_ptr<Model> model;
    };
}