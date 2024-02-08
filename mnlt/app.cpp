#include "app.hpp"
#include "camera.hpp"
#include "simple_render_system.hpp"
#include "input_controller.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <chrono>

namespace mnlt
{
    App::App()
    {
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
            void update(std::vector<GameObject>& objs, float dt, unsigned int substeps = 1) {
                const float stepDelta = dt / substeps;
                for (int i = 0; i < substeps; i++) 
                {
                    stepSimulation(objs, stepDelta);
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
            void stepSimulation(std::vector<GameObject>& physicsObjs, float dt) 
            {
                // Loops through all pairs of objects and applies attractive force between them
                for (auto iterA = physicsObjs.begin(); iterA != physicsObjs.end(); ++iterA) 
                {
                    auto& objA = *iterA;
                    for (auto iterB = iterA; iterB != physicsObjs.end(); ++iterB) 
                    {
                        if (iterA == iterB) continue;
                        auto& objB = *iterB;
                
                        auto force = computeForce(objA, objB);
                        objA.rigidBody.velocity += dt * -force / objA.rigidBody.mass;
                        objB.rigidBody.velocity += dt * force / objB.rigidBody.mass;
                    }
                }
            
                // update each objects position based on its final velocity
                for (auto& obj : physicsObjs) 
                {
                    obj.transform.translation += dt * obj.rigidBody.velocity;
                }
            }
    };

    void App::run()
    {
        SimpleRenderSystem simpleRenderSystem{device, renderer.getSwapChainRenderPass()};
        Camera camera{};

        auto viewerObject = GameObject::createGameObject();
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
                //gravitySystem.update(gameObjects, 1.f / 60, 5);
                renderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
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
        sun.model = Model::createModelFromFile(device, "models/colored_cube.obj"); // Use a more realistic model
        gameObjects.push_back(std::move(sun));

        // Create Planets
        // Mercury
        auto mercury = GameObject::createGameObject();
        mercury.transform.scale = glm::vec3{0.05f}; // Adjust scale
        mercury.transform.translation = {4.0f, 0.0f, 0.0f}; // Position relative to the sun
        mercury.color = {0.7f, 0.7f, 0.7f}; // Gray color for Mercury
        mercury.rigidBody.velocity = {.0f, .0f, 7.0f}; // Adjust initial velocity
        mercury.rigidBody.mass = 0.055f; // Adjust mass relative to Earth
        mercury.model = Model::createModelFromFile(device, "models/colored_cube.obj"); // Use a specific model for Mercury
        gameObjects.push_back(std::move(mercury));

        // Venus
        auto venus = GameObject::createGameObject();
        venus.transform.scale = glm::vec3{0.08f}; // Adjust scale
        venus.transform.translation = {6.0f, 0.0f, 0.0f}; // Position relative to the sun
        venus.color = {0.8f, 0.5f, 0.1f}; // Brownish color for Venus
        venus.rigidBody.velocity = {.0f, .0f, 5.0f}; // Adjust initial velocity
        venus.rigidBody.mass = 0.815f; // Adjust mass relative to Earth
        venus.model = Model::createModelFromFile(device, "models/colored_cube.obj"); // Use a specific model for Venus
        gameObjects.push_back(std::move(venus));

        // Earth
        auto earth = GameObject::createGameObject();
        earth.transform.scale = glm::vec3{0.1f}; // Adjust scale
        earth.transform.translation = {9.0f, 0.0f, 0.0f}; // Position relative to the sun
        earth.color = {0.0f, 0.6f, 1.0f}; // Blue color for Earth
        earth.rigidBody.velocity = {.0f, .0f, 3.0f}; // Adjust initial velocity
        earth.rigidBody.mass = 1.0f; // Mass of Earth
        earth.model = Model::createModelFromFile(device, "models/colored_cube.obj"); // Use a specific model for Earth
        gameObjects.push_back(std::move(earth));

        // Mars
        auto mars = GameObject::createGameObject();
        mars.transform.scale = glm::vec3{0.08f}; // Adjust scale
        mars.transform.translation = {12.0f, 0.0f, 0.0f}; // Position relative to the sun
        mars.color = {1.0f, 0.3f, 0.0f}; // Reddish color for Mars
        mars.rigidBody.velocity = {.0f, .0f, 2.5f}; // Adjust initial velocity
        mars.rigidBody.mass = 0.107f; // Adjust mass relative to Earth
        mars.model = Model::createModelFromFile(device, "models/colored_cube.obj"); // Use a specific model for Mars
        gameObjects.push_back(std::move(mars));
    }

    void App::loadGameObjects() 
    {
        std::shared_ptr<Model> model = Model::createModelFromFile(device, "models/smooth_vase.obj");
        auto cube = GameObject::createGameObject();
        cube.model = model;
        cube.transform.translation = {.0f, .0f, 2.5f};
        cube.transform.scale = glm::vec3(3.f);
        gameObjects.push_back(std::move(cube));
    }
}