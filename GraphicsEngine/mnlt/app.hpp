#pragma once

#include "window.hpp"
#include "device.hpp"
#include "pipeline.hpp"

namespace mnlt
{
    class App
    {
        public:
            constexpr static int WIDTH = 800;
            constexpr static int HEIGHT = 600;

            void run();

        private:
            Window window{WIDTH, HEIGHT, "MoonLight"};
            Device device{window};
            Pipeline pipeline{device, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", Pipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
    };
}