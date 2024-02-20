#pragma once

#include "mnlt/app.hpp"
#include "mnlt/game_object.hpp"
#include "mnlt/model.hpp"
#include "mnlt/render_systems/3d_grid_system.hpp"
#include "mnlt/render_systems/point_light_system.hpp"
#include "mnlt/render_systems/simple_render_system.hpp"
#include "mnlt/ui.hpp"


class PartcleType
{
    public:
        PartcleType(std::string name, glm::vec3 color, int numOfParticles)
         : name{name}, color{color}, numOfParticles{numOfParticles}
        {
            id = nextId++;
        }
        std::string name = "Particle " + std::to_string(getId());
        glm::vec3 color;
        int numOfParticles;
        std::vector<mnlt::GameObject*> particles;
        std::vector<float> attraction;
        int getId() const { return id; }

        bool showPropertyWindow = false;

    private:
        static inline int nextId;
        int id;
};
class ParticleLifeSystem
{
    public:
        ParticleLifeSystem(std::shared_ptr<mnlt::Model> model, glm::vec3 lowerBound, glm::vec3 upperBound) : model{model}, lowerBound{lowerBound}, upperBound{upperBound} {}
        void createParticles(mnlt::GameObject::Map* particleObjects);
        void createParticleLifeUI(mnlt::GameObject::Map* particleObjects);
        glm::vec3 random3DPosition(glm::vec3 lowerBound, glm::vec3 upperBound);
        float randomFloat(float min, float max);
        void updateParticleLife(mnlt::Time time);
        void particleTypePhysics(PartcleType& type1, PartcleType& type2, float deltaTime);

        std::vector<PartcleType> particleTypes;
        
    private:
        bool isBounded = true;
        float viscosity = 0.1f;
        float radiusOfAttraction = 0.5;
        glm::vec3 lowerBound;
        glm::vec3 upperBound;
        std::shared_ptr<mnlt::Model> model;
};
class PartcleLife : public mnlt::App 
{
    public:
        // Override functions that are declared to be in child class
        void start() override;
        void renderSystems(VkCommandBuffer commandBuffer, mnlt::FrameInfo frameInfo) override;
        void update(mnlt::Time time) override;

    private:
        ParticleLifeSystem particleLifeSystem{mnlt::Model::createModelFromFile(device, "assets/models/sphere.obj"), {-1.f,-1.f,-1.f}, {1.f,1.f,1.f}};

        mnlt::SimpleRenderSystem simpleRenderSystem{device};
        mnlt::PointLightSystem pointLightSystem{device};
        mnlt::GridSystem gridSystem{device};
        mnlt::UI ui{window, device};
};