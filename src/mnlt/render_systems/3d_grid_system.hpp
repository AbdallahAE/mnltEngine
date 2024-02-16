#pragma once

#include "../device.hpp"
#include "../pipeline.hpp"
#include "../frame_info.hpp"

// std
#include <memory>

namespace mnlt
{
    class GridSystem 
    {
        public:
            GridSystem(Device &device);
            ~GridSystem();

            GridSystem(const GridSystem &) = delete;
            GridSystem &operator=(const GridSystem &) = delete;

            void createRenderer(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            void render(FrameInfo &frameInfo);

        private:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createPipeline(VkRenderPass renderPass);

            Device &device;

            std::unique_ptr<Pipeline> pipeline;
            VkPipelineLayout pipelineLayout;
    };
}