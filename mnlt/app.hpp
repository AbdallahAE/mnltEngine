#pragma once

#include "window.hpp"
#include "device.hpp"
#include "game_object.hpp"
#include "renderer.hpp"


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

            Window window{WIDTH, HEIGHT, "MoonLight"};
            Device device{window};
            Renderer renderer{window, device};

            std::vector<GameObject> gameObjects;
    };
}