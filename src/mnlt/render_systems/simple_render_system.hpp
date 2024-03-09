#pragma once

#include "../device.hpp"
#include "../pipeline.hpp"
#include "../frame_info.hpp"

// std
#include <memory>

namespace mnlt
{
    class SimpleRenderSystem 
    {
        public:
            SimpleRenderSystem(Device &device);
            ~SimpleRenderSystem();

            SimpleRenderSystem(const SimpleRenderSystem &) = delete;
            SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

            void createRenderer(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            void renderGameObjects(FrameInfo &frameInfo);

        private:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createPipeline(VkRenderPass renderPass);

            Device &device;

            std::unique_ptr<Pipeline> pipeline;
            VkPipelineLayout pipelineLayout;

            std::unique_ptr<DescriptorSetLayout> renderSystemLayout;
    };
}
