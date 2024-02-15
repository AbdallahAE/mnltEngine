#include "app.hpp"
#include "camera.hpp"
#include "frame_info.hpp"
#include "render_systems/3d_grid_system.hpp"
#include "render_systems/simple_render_system.hpp"
#include "render_systems/point_light_system.hpp"
#include "input_controller.hpp"
#include "buffer.hpp"
#include "ui.hpp"
#include "time.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace mnlt
{
    App::App()
    {
        globalPool = DescriptorPool::Builder(device)
          .setMaxSets(11000)
          .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1000)
          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
          .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000)
          .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
          .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
          .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
          .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
          .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000)
          .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
          .build();
        
        loadPhysics();
        //loadGameObjects();
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
            void update(FrameInfo frameInfo) 
            {
                    stepSimulation(&frameInfo.gameObjects, frameInfo.time.getDeltaTime());
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
            void stepSimulation(GameObject::Map* physicsObjs, float deltaTime) 
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
                        objA.second.rigidBody.velocity += deltaTime * -force / objA.second.rigidBody.mass;
                        objB.second.rigidBody.velocity += deltaTime * force / objB.second.rigidBody.mass;
                    }
                }
            
                // update each objects position based on its final velocity
                for (auto& obj : *physicsObjs) 
                {
                    obj.second.transform.translation += deltaTime * obj.second.rigidBody.velocity;
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

        UI ui
        {
            window,
            device,
            renderer.getSwapChainRenderPass(),
            renderer.getImageCount(),
            globalPool->getDescriptorPool()
        };
        Camera camera{};

        auto viewerObject = GameObject::createGameObject();
        viewerObject.transform.translation = {0.0f, -1.0f, -5.0f};
        CameraController cameraController{};

        SimpleRenderSystem simpleRenderSystem{device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        PointLightSystem pointLightSystem{device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        GridSystem gridSystem{device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};

        Time time;

        GravityPhysicsSystem gravitySystem{6.674e-4f};

        while (!window.shouldClose())
        {
            glfwPollEvents();

            time.update();

            cameraController.moveInPlaneXZ(window.getGLFWwindow(), time.getPureDeltaTime(), viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);
            
            if (auto commandBuffer = renderer.beginFrame()) 
            {
                int frameIndex = renderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, time, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects};
                                
                gravitySystem.update(frameInfo);

                // update
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                ui.newFrame();

                // render
                renderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);
                if(camera.enableGrid)
                    gridSystem.render(frameInfo);

                ui.runExample(frameInfo);
                ui.render(commandBuffer);

                renderer.endSwapChainRenderPass(commandBuffer);
                renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(device.device());
    }

    void App::loadPhysics()
    {
        // Create the Sun
        auto sun = GameObject::makePointLight(10.f);
        sun.transform.scale = glm::vec3{0.2f}; // Increase the size for better visualization
        sun.transform.translation = {0.0f, 0.0f, 0.0f};
        sun.color = {1.f, 1.f, 0.f}; // Yellow color for the sun
        sun.rigidBody.velocity = {.0f, .0f, .0f};
        sun.rigidBody.mass = 333000.f; // Sun's mass is significantly higher
        gameObjects.emplace(sun.getId(), std::move(sun));

        // Create Planets
        // Mercury
        auto mercury = GameObject::makePointLight(10.f);
        mercury.transform.scale = glm::vec3{0.05f}; // Adjust scale
        mercury.transform.translation = {4.0f, 0.0f, 0.0f}; // Position relative to the sun
        mercury.color = {0.7f, 0.7f, 0.7f}; // Gray color for Mercury
        mercury.rigidBody.velocity = {.0f, .0f, 7.0f}; // Adjust initial velocity
        mercury.rigidBody.mass = 0.055f; // Adjust mass relative to Earth
        gameObjects.emplace(mercury.getId(), std::move(mercury));

        // Venus
        auto venus = GameObject::makePointLight(10.f);
        venus.transform.scale = glm::vec3{0.08f}; // Adjust scale
        venus.transform.translation = {6.0f, 0.0f, 0.0f}; // Position relative to the sun
        venus.color = {0.8f, 0.5f, 0.1f}; // Brownish color for Venus
        venus.rigidBody.velocity = {.0f, .0f, 5.0f}; // Adjust initial velocity
        venus.rigidBody.mass = 0.815f; // Adjust mass relative to Earth
        gameObjects.emplace(venus.getId(), std::move(venus));

        // Earth
        auto earth = GameObject::makePointLight(10.f);
        earth.transform.scale = glm::vec3{0.1f}; // Adjust scale
        earth.transform.translation = {9.0f, 0.0f, 0.0f}; // Position relative to the sun
        earth.color = {0.0f, 0.6f, 1.0f}; // Blue color for Earth
        earth.rigidBody.velocity = {.0f, .0f, 3.0f}; // Adjust initial velocity
        earth.rigidBody.mass = 1.0f; // Mass of Earth
        gameObjects.emplace(earth.getId(), std::move(earth));

        // Mars
        auto mars = GameObject::makePointLight(10.f);
        mars.transform.scale = glm::vec3{0.08f}; // Adjust scale
        mars.transform.translation = {12.0f, 0.0f, 0.0f}; // Position relative to the sun
        mars.color = {1.0f, 0.3f, 0.0f}; // Reddish color for Mars
        mars.rigidBody.velocity = {.0f, .0f, 2.5f}; // Adjust initial velocity
        mars.rigidBody.mass = 0.107f; // Adjust mass relative to Earth
        gameObjects.emplace(mars.getId(), std::move(mars));
    }

    void App::loadGameObjects() 
    {
        std::shared_ptr<Model> model = Model::createModelFromFile(device, "assets/models/colored_cube.obj");
        auto cube = GameObject::createGameObject();
        cube.model = model;
        cube.transform.translation = {-0.5f, -0.5f, 0.0f};
        cube.transform.scale = {0.2f, 0.2f, 0.2f};
        cube.name = "cube";
        gameObjects.emplace(cube.getId(), std::move(cube));

        model = Model::createModelFromFile(device, "assets/models/smooth_vase.obj");
        auto smoothVase = GameObject::createGameObject();
        smoothVase.model = model;
        smoothVase.transform.translation = {0.5f, 0.0f, 0.0f};
        smoothVase.transform.scale = {3.0f, 2.5f, 3.0f};
        smoothVase.name = "smoothvase";
        gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

        model = Model::createModelFromFile(device, "assets/models/quad.obj");
        auto floor = GameObject::createGameObject();
        floor.model = model;
        floor.transform.translation = {0.0f, 0.0f, 0.0f};
        floor.transform.scale = {3.0f, 1.0f, 3.0f};
        floor.name = "floor";
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