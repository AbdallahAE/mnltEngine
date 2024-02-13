#pragma once

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

        private:
            void loadGameObjects();
            void loadPhysics();

            Window window{WIDTH, HEIGHT, "MoonLight"};
            Device device{window};
            Renderer renderer{window, device};

            // note: order of declarations matters
            std::unique_ptr<DescriptorPool> globalPool{};
            GlobalUbo ubo;
            GameObject::Map gameObjects;
    };
}