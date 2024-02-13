#include "app.hpp"
#include "camera.hpp"
#include "frame_info.hpp"
#include "render_systems/simple_render_system.hpp"
#include "render_systems/point_light_system.hpp"
#include "input_controller.hpp"
#include "buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <chrono>

namespace mnlt
{
    App::App()
    {
        globalPool = DescriptorPool::Builder(device)
          .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
          .build();
        
        //loadPhysics();
        loadGameObjects();
    }
    App::~App()
    {
        
    }
    class GravityPhysicsSystem
    {
        public:
            GravityPhysicsSystem(float strength) : strengthGravity{strength} {}
            
            const float strengthGravity;
            
            // dt stands for delta time, and specifies the amount of time to advance the simulation
            // substeps is how many intervals to divide the forward time step in. More substeps result in a
            // more stable simulation, but takes longer to compute
            void update(FrameInfo frameInfo, float dt, unsigned int substeps = 1) {
                const float stepDelta = dt / substeps;
                for (int i = 0; i < substeps; i++) 
                {
                    stepSimulation(&frameInfo.gameObjects, stepDelta);
                }
            }
            
            glm::vec3 computeForce(GameObject& fromObj, GameObject& toObj) const {
                auto offset = fromObj.transform.translation - toObj.transform.translation;
                float distanceSquared = glm::dot(offset, offset);
            
                // clown town - just going to return 0 if objects are too close together...
                if (glm::abs(distanceSquared) < 0.1f) 
                {
                    return {0.0f, 0.0f, 0.0f};
                }
            
                float force = strengthGravity * toObj.rigidBody.mass * fromObj.rigidBody.mass / distanceSquared;
                return force * offset / glm::sqrt(distanceSquared);
            }
        
        private:
            void stepSimulation(GameObject::Map* physicsObjs, float dt) 
            {
                // Loops through all pairs of objects and applies attractive force between them
                for (auto iterA = physicsObjs->begin(); iterA != physicsObjs->end(); ++iterA) 
                {
                    auto& objA = *iterA;
                    for (auto iterB = iterA; iterB != physicsObjs->end(); ++iterB) 
                    {
                        if (iterA == iterB) continue;
                        auto& objB = *iterB;
                
                        auto force = computeForce(objA.second, objB.second);
                        objA.second.rigidBody.velocity += dt * -force / objA.second.rigidBody.mass;
                        objB.second.rigidBody.velocity += dt * force / objB.second.rigidBody.mass;
                    }
                }
            
                // update each objects position based on its final velocity
                for (auto& obj : *physicsObjs) 
                {
                    obj.second.transform.translation += dt * obj.second.rigidBody.velocity;
                }
            }
    };

    void App::run()
    {
        std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) 
        {
            uboBuffers[i] = std::make_unique<Buffer>
            (
                device,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            uboBuffers[i]->map();
        }

        auto globalSetLayout = DescriptorSetLayout::Builder(device)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) 
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            DescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        SimpleRenderSystem simpleRenderSystem{device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        PointLightSystem pointLightSystem{device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        Camera camera{};

        auto viewerObject = GameObject::createGameObject();
        viewerObject.transform.translation = {0.0f, -1.0f, -5.0f};
        CameraController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        //GravityPhysicsSystem gravitySystem{6.674e-4f};

        while (!window.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(window.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);
            
            if (auto commandBuffer = renderer.beginFrame()) 
            {
                int frameIndex = renderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects};
                                
                //gravitySystem.update(frameInfo, 1.f / 60, 5);

                // update
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                renderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);
                renderer.endSwapChainRenderPass(commandBuffer);
                renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(device.device());
    }

    void App::loadPhysics()
    {
        // Create the Sun
        auto sun = GameObject::createGameObject();
        sun.transform.scale = glm::vec3{0.2f}; // Increase the size for better visualization
        sun.transform.translation = {0.0f, 0.0f, 0.0f};
        sun.color = {1.f, 1.f, 0.f}; // Yellow color for the sun
        sun.rigidBody.velocity = {.0f, .0f, .0f};
        sun.rigidBody.mass = 333000.f; // Sun's mass is significantly higher
        sun.model = Model::createModelFromFile(device, "assets/models/colored_cube.obj"); // Use a more realistic model
        gameObjects.emplace(sun.getId(), std::move(sun));

        // Create Planets
        // Mercury
        auto mercury = GameObject::createGameObject();
        mercury.transform.scale = glm::vec3{0.05f}; // Adjust scale
        mercury.transform.translation = {4.0f, 0.0f, 0.0f}; // Position relative to the sun
        mercury.color = {0.7f, 0.7f, 0.7f}; // Gray color for Mercury
        mercury.rigidBody.velocity = {.0f, .0f, 7.0f}; // Adjust initial velocity
        mercury.rigidBody.mass = 0.055f; // Adjust mass relative to Earth
        mercury.model = Model::createModelFromFile(device, "assets/models/colored_cube.obj"); // Use a specific model for Mercury
        gameObjects.emplace(mercury.getId(), std::move(mercury));

        // Venus
        auto venus = GameObject::createGameObject();
        venus.transform.scale = glm::vec3{0.08f}; // Adjust scale
        venus.transform.translation = {6.0f, 0.0f, 0.0f}; // Position relative to the sun
        venus.color = {0.8f, 0.5f, 0.1f}; // Brownish color for Venus
        venus.rigidBody.velocity = {.0f, .0f, 5.0f}; // Adjust initial velocity
        venus.rigidBody.mass = 0.815f; // Adjust mass relative to Earth
        venus.model = Model::createModelFromFile(device, "assets/models/colored_cube.obj"); // Use a specific model for Venus
        gameObjects.emplace(venus.getId(), std::move(venus));

        // Earth
        auto earth = GameObject::createGameObject();
        earth.transform.scale = glm::vec3{0.1f}; // Adjust scale
        earth.transform.translation = {9.0f, 0.0f, 0.0f}; // Position relative to the sun
        earth.color = {0.0f, 0.6f, 1.0f}; // Blue color for Earth
        earth.rigidBody.velocity = {.0f, .0f, 3.0f}; // Adjust initial velocity
        earth.rigidBody.mass = 1.0f; // Mass of Earth
        earth.model = Model::createModelFromFile(device, "assets/models/colored_cube.obj"); // Use a specific model for Earth
        gameObjects.emplace(earth.getId(), std::move(earth));

        // Mars
        auto mars = GameObject::createGameObject();
        mars.transform.scale = glm::vec3{0.08f}; // Adjust scale
        mars.transform.translation = {12.0f, 0.0f, 0.0f}; // Position relative to the sun
        mars.color = {1.0f, 0.3f, 0.0f}; // Reddish color for Mars
        mars.rigidBody.velocity = {.0f, .0f, 2.5f}; // Adjust initial velocity
        mars.rigidBody.mass = 0.107f; // Adjust mass relative to Earth
        mars.model = Model::createModelFromFile(device, "assets/models/colored_cube.obj"); // Use a specific model for Mars
        gameObjects.emplace(mars.getId(), std::move(mars));
    }

    void App::loadGameObjects() 
    {
        std::shared_ptr<Model> model = Model::createModelFromFile(device, "assets/models/sphere.obj");
        auto flatVase = GameObject::createGameObject();
        flatVase.model = model;
        flatVase.transform.translation = {-0.5f, -0.5f, 0.0f};
        flatVase.transform.scale = {0.2f, 0.2f, 0.2f};
        gameObjects.emplace(flatVase.getId(), std::move(flatVase));

        model = Model::createModelFromFile(device, "assets/models/smooth_vase.obj");
        auto smoothVase = GameObject::createGameObject();
        smoothVase.model = model;
        smoothVase.transform.translation = {0.5f, 0.0f, 0.0f};
        smoothVase.transform.scale = {3.0f, 2.5f, 3.0f};
        gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

        model = Model::createModelFromFile(device, "assets/models/quad.obj");
        auto floor = GameObject::createGameObject();
        floor.model = model;
        floor.transform.translation = {0.0f, 0.0f, 0.0f};
        floor.transform.scale = {3.0f, 1.0f, 3.0f};
        gameObjects.emplace(floor.getId(), std::move(floor));

        std::vector<glm::vec3> lightColors
        {
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f}
        };

        for (int i = 0; i < lightColors.size(); i++) 
        {
            auto pointLight = GameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                {0.f, -1.f, 0.f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
    }
}