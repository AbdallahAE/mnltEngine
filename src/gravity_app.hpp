#pragma once

#include "mnlt/app.hpp"
#include "mnlt/render_systems/3d_grid_system.hpp"
#include "mnlt/render_systems/point_light_system.hpp"
#include "mnlt/render_systems/simple_render_system.hpp"
#include "mnlt/ui.hpp"

class GravityPhysicsSystem
{
    public:
        GravityPhysicsSystem(float strength) : strengthGravity{strength} {}
        const float strengthGravity;
        void update(mnlt::GameObject::Map* physicsObjs, mnlt::Time time, int substeps);
        glm::vec3 computeForce(mnlt::GameObject& fromObj, mnlt::GameObject& toObj) const;
        
    private:
        void stepSimulation(mnlt::GameObject::Map* physicsObjs, float deltaTime);
};
class GravityApp : public mnlt::App 
{
    public:
        void start() override;
        void renderSystems(VkCommandBuffer commandBuffer, mnlt::FrameInfo frameInfo) override;
        void update(mnlt::Time time) override;

    private:
        void loadPhysicsObjects();

        GravityPhysicsSystem gravitySystem{6.674e-18f};


        mnlt::SimpleRenderSystem simpleRenderSystem{device};
        mnlt::PointLightSystem pointLightSystem{device};
        mnlt::GridSystem gridSystem{device};
        mnlt::UI ui{window, device};
};
