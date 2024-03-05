#pragma once

#include "camera.hpp"
#include "frame_info.hpp"
#include "window.hpp"
#include "device.hpp"
#include "game_object.hpp"
#include "renderer.hpp"
#include "descriptors.hpp"


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

        protected:
            virtual void start() = 0;
            virtual void renderSystems(VkCommandBuffer commandBuffer, FrameInfo frameInfo) = 0;
            virtual void update(Time time) = 0;

            Window window{WIDTH, HEIGHT, "MoonLight"};
            Device device{window};
            Renderer renderer{window, device};
            Camera camera{};
            std::unique_ptr<DescriptorSetLayout> globalSetLayout;

            // note: order of declarations matters
            std::unique_ptr<DescriptorPool> globalPool;
            GlobalUbo ubo;
            std::vector<std::unique_ptr<DescriptorPool>> framePools;
            GameObjectManager gameObjectManager{device};
    };
}