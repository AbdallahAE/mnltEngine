#include "particle_life.hpp"

#include "../libs/imgui/imgui.h"
#include "mnlt/game_object.hpp"

#include <glm/common.hpp>
#include <random>

void PartcleLife::start()
{
    ubo.ambientLightColor = glm::vec4(1.f);
    camera.setPerspectiveProjection(glm::radians(50.f), renderer.getAspectRatio(), 0.1f, 1000.f);

    ui.initialize(renderer.getSwapChainRenderPass(), renderer.getImageCount(), globalPool->getDescriptorPool());
    simpleRenderSystem.createRenderer(renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
    pointLightSystem.createRenderer(renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
    gridSystem.createRenderer(renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());

    PartcleType p1{"red", {1.f, 0.f, 0.f}, 50};
    particleLifeSystem.particleTypes.push_back(p1);
    PartcleType p2{"green", {0.f, 1.f, 0.f}, 50};
    particleLifeSystem.particleTypes.push_back(p2);
    PartcleType p3{"blue", {0.f, 0.f, 1.f}, 50};
    particleLifeSystem.particleTypes.push_back(p3);
    PartcleType p4{"white", {1.f, 1.f, 1.f}, 50};
    particleLifeSystem.particleTypes.push_back(p4);
    particleLifeSystem.createParticles(&gameObjectManager);
}
void PartcleLife::renderSystems(VkCommandBuffer commandBuffer, mnlt::FrameInfo frameInfo)
{
    simpleRenderSystem.renderGameObjects(frameInfo);
    pointLightSystem.renderLights(frameInfo, ubo);
    if(camera.enableGrid)
        gridSystem.render(frameInfo);
    ui.newFrame();
    ui.runExample(frameInfo);
    particleLifeSystem.createParticleLifeUI(&gameObjectManager);
    ui.render(commandBuffer);
}
void PartcleLife::update(mnlt::Time time)
{
    camera.setPerspectiveProjection(glm::radians(50.f), renderer.getAspectRatio(), 0.1f, 1000.f);
    camera.move(window.getGLFWwindow(), time.getPureDeltaTime());

    particleLifeSystem.updateParticleLife(time);
}

void ParticleLifeSystem::createParticles(mnlt::GameObjectManager* particleObjectsManager)
{
    for(auto& pt : particleTypes)
    {
        for(auto& paty : particleTypes)
        {
            pt.attraction.push_back(randomFloat(-1.f, 1.f));
        }
        for(int i=0; i<pt.numOfParticles; i++)
        {
            auto& obj = particleObjectsManager->createGameObject();
            obj.color = pt.color;
            obj.model = model;
            obj.transform.scale = glm::vec3(0.01f);
            obj.transform.translation = random3DPosition(lowerBound, upperBound);
            obj.rigidBody.mass = randomFloat(0, 100);
         
            pt.particles.push_back(&obj);
        }
    }
}
void ParticleLifeSystem::updateParticleLife(mnlt::Time time) 
{
    for (auto& type1 : particleTypes) 
    {
        for (auto& type2 : particleTypes) 
        {
            particleTypePhysics(type1, type2, time.getDeltaTime());
        }
    }
    
}
void ParticleLifeSystem::particleTypePhysics(PartcleType& type1, PartcleType& type2, float deltaTime)
{
    float g = type1.attraction[type2.getId()] / -100;
    
    for(auto& p1 : type1.particles)
    {
        glm::vec3 totalForce{0};

        for(auto& p2 : type2.particles)
        {
            glm::vec3 direction = p1->transform.translation - p2->transform.translation;
            float distanceSquared = glm::dot(direction, direction);

            if(p1->transform.translation != p2->transform.translation)
            {
                float force = distanceSquared < radiusOfAttraction*radiusOfAttraction ? (1.0f / sqrt(distanceSquared) * p1->rigidBody.mass * p2->rigidBody.mass): 0.0f;
                totalForce += direction * force;
            }
        }

        p1->rigidBody.velocity = deltaTime * (p1->rigidBody.velocity + (totalForce * g)) * (1.0f - viscosity);
        p1->transform.translation += deltaTime * p1->rigidBody.velocity;
    }

    if(isBounded)
    {
        for(auto& p1 : type1.particles)
        {
            p1->transform.translation = glm::min(glm::max(p1->transform.translation, lowerBound), upperBound);
        }
    }
}

void ParticleLifeSystem::createParticleLifeUI(mnlt::GameObjectManager* particleObjectsManager)
{
    ImGui::Begin("Particle Life");
    ImGui::Checkbox("Bounded", &isBounded);
    ImGui::DragFloat("Radius", &radiusOfAttraction, 0.01f, 0.f, 10.f);
    ImGui::DragFloat("Viscosity", &viscosity, 0.01f, 0.1f, 2.f);
    static int selectedpt;
    for(auto& pt : particleTypes)
    {
        std::string str = "Particle Type " + pt.name;
        bool isSelected = (pt.getId() == selectedpt);

        if (ImGui::Selectable(str.c_str(), isSelected)) {
            // Update the selected GameObject name
            selectedpt = pt.getId();
        }

        // Open the property window for the selected GameObject
        if (isSelected) {
            pt.showPropertyWindow = true;
            ImGui::Begin("Particle Life Properties");
            ImGui::Text("Number of Particles:");
            ImGui::DragInt("Number of Particles", &pt.numOfParticles);
            ImGui::Text("Particle's Color:");
            ImGui::ColorEdit3("Color", &pt.color[0]);
            for(int i=0; i<pt.attraction.size(); i++)
            {
                std::string atractLabel = "Attraction " + particleTypes[i].name; 
                ImGui::Text("Particle %s Forces:", particleTypes[i].name.c_str());
                ImGui::DragFloat(atractLabel.c_str(), &pt.attraction.at(i), 0.01f, -1.f, 1.f);
            }
            if(ImGui::Button("Update"))
            {
                if(pt.particles.size() != pt.numOfParticles)
                {
                    particleObjectsManager->gameObjects.clear();
                    for(auto& paty : particleTypes)
                    {
                        paty.particles.clear();
                        paty.attraction.clear();
                    }
                    createParticles(particleObjectsManager);
                }
                for(auto &p : pt.particles)
                {
                    p->color = pt.color;
                }
            }
            ImGui::End();
        } else {
            pt.showPropertyWindow = false;
        }
    }
    ImGui::End();
}
float ParticleLifeSystem::randomFloat(float min, float max)
{
    static std::random_device rd;
    static std::default_random_engine eng(rd());

    std::uniform_real_distribution<float> dist(min, max);

    return dist(eng);

}
glm::vec3 ParticleLifeSystem::random3DPosition(glm::vec3 lowerBound, glm::vec3 upperBound)
{
        // Create a random engine
    static std::random_device rd;
    static std::default_random_engine eng(rd());

    // Create distributions for each axis
    std::uniform_real_distribution<float> distX(lowerBound.x, upperBound.x);
    std::uniform_real_distribution<float> distY(lowerBound.y, upperBound.y);
    std::uniform_real_distribution<float> distZ(lowerBound.z, upperBound.z);

    // Generate a random position within the specified bounds
    glm::vec3 randomPos;
    randomPos.x = distX(eng);
    randomPos.y = distY(eng);
    randomPos.z = distZ(eng);

    return randomPos;
}
