#pragma once

#include "../device.hpp"
#include "../frame_info.hpp"
#include "../pipeline.hpp"

// std
#include <memory>

namespace mnlt
{
    class PointLightSystem
    {
        public:
            PointLightSystem(Device &device);
            ~PointLightSystem();

            PointLightSystem(const PointLightSystem &) = delete;
            PointLightSystem &operator=(const PointLightSystem &) = delete;

            void createRenderer(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            void renderLights(FrameInfo &frameInfo, GlobalUbo &ubo);

        private:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createPipeline(VkRenderPass renderPass);

            Device &device;

            std::unique_ptr<Pipeline> pipeline;
            VkPipelineLayout pipelineLayout;
    };
}