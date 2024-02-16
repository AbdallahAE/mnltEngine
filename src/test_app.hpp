#pragma once

#include "mnlt/app.hpp"
#include "mnlt/render_systems/3d_grid_system.hpp"
#include "mnlt/render_systems/point_light_system.hpp"
#include "mnlt/render_systems/simple_render_system.hpp"
#include "mnlt/ui.hpp"


class TestApp : public mnlt::App 
{
    public:
        // Override functions that are declared to be in child class
        void start() override;
        void renderSystems(VkCommandBuffer commandBuffer, mnlt::FrameInfo frameInfo) override;
        void update(mnlt::Time time) override;

    private:
        void loadGameObjects();
        void loadPhysics();

        mnlt::SimpleRenderSystem simpleRenderSystem{device};
        mnlt::PointLightSystem pointLightSystem{device};
        mnlt::GridSystem gridSystem{device};
        mnlt::UI ui{window, device};
};