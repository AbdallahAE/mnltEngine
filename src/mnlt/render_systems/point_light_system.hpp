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
            PointLightSystem(
                Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            ~PointLightSystem();

            PointLightSystem(const PointLightSystem &) = delete;
            PointLightSystem &operator=(const PointLightSystem &) = delete;

            void render(FrameInfo &frameInfo);

        private:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createPipeline(VkRenderPass renderPass);

            Device &device;

            std::unique_ptr<Pipeline> pipeline;
            VkPipelineLayout pipelineLayout;
    };
}