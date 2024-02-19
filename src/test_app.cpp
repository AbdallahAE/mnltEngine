#include "test_app.hpp"
#include "mnlt/game_object.hpp"

class GravityPhysicsSystem
{
    public:
        GravityPhysicsSystem(float strength) : strengthGravity{strength} {}
        
        const float strengthGravity;
        
        // dt stands for delta time, and specifies the amount of time to advance the simulation
        // substeps is how many intervals to divide the forward time step in. More substeps result in a
        // more stable simulation, but takes longer to compute
        void update(mnlt::GameObject::Map* physicsObjs, mnlt::Time time, int substeps) 
        {
            for(int i=0; i<substeps; i++)
                stepSimulation(physicsObjs, time.getDeltaTime() / substeps);
        }
        
        glm::vec3 computeForce(mnlt::GameObject& fromObj, mnlt::GameObject& toObj) const {
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
        void stepSimulation(mnlt::GameObject::Map* physicsObjs, float deltaTime) 
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
GravityPhysicsSystem gravitySystem{6.674e-4f};
void TestApp::start()
{
    camera.setPerspectiveProjection(glm::radians(50.f), renderer.getAspectRatio(), 0.1f, 1000.f);

    ui.initialize(renderer.getSwapChainRenderPass(), renderer.getImageCount(), globalPool->getDescriptorPool());
    simpleRenderSystem.createRenderer(renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
    pointLightSystem.createRenderer(renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
    gridSystem.createRenderer(renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());

    loadGameObjects();
    //loadPhysics();
}
void TestApp::renderSystems(VkCommandBuffer commandBuffer, mnlt::FrameInfo frameInfo)
{
    simpleRenderSystem.renderGameObjects(frameInfo);
    pointLightSystem.renderLights(frameInfo, ubo);
    if(camera.enableGrid)
        gridSystem.render(frameInfo);
    ui.newFrame();
    ui.runExample(frameInfo);
    ui.render(commandBuffer);
}
void TestApp::update(mnlt::Time time)
{
    if(window.wasWindowResized())
        camera.setPerspectiveProjection(glm::radians(50.f), renderer.getAspectRatio(), 0.1f, 1000.f);
    camera.move(window.getGLFWwindow(), time.getPureDeltaTime());
    
    //gravitySystem.update(&gameObjects, time, 100);
}


void TestApp::loadPhysics()
{
    // Create the Sun
    auto sun = mnlt::GameObject::makePointLight(10.f);
    sun.transform.scale = glm::vec3{0.2f}; // Increase the size for better visualization
    sun.transform.translation = {0.0f, 0.0f, 0.0f};
    sun.color = {1.f, 1.f, 0.f}; // Yellow color for the sun
    sun.rigidBody.velocity = {.0f, .0f, .0f};
    sun.rigidBody.mass = 333000.f; // Sun's mass is significantly higher
    gameObjects.emplace(sun.getId(), std::move(sun));
    // Create Planets
    // Mercury
    auto mercury = mnlt::GameObject::makePointLight(10.f);
    mercury.transform.scale = glm::vec3{0.05f}; // Adjust scale
    mercury.transform.translation = {4.0f, 0.0f, 0.0f}; // Position relative to the sun
    mercury.color = {0.7f, 0.7f, 0.7f}; // Gray color for Mercury
    mercury.rigidBody.velocity = {.0f, .0f, 7.0f}; // Adjust initial velocity
    mercury.rigidBody.mass = 0.055f; // Adjust mass relative to Earth
    gameObjects.emplace(mercury.getId(), std::move(mercury));
    // Venus
    auto venus = mnlt::GameObject::makePointLight(10.f);
    venus.transform.scale = glm::vec3{0.08f}; // Adjust scale
    venus.transform.translation = {6.0f, 0.0f, 0.0f}; // Position relative to the sun
    venus.color = {0.8f, 0.5f, 0.1f}; // Brownish color for Venus
    venus.rigidBody.velocity = {.0f, .0f, 5.0f}; // Adjust initial velocity
    venus.rigidBody.mass = 0.815f; // Adjust mass relative to Earth
    gameObjects.emplace(venus.getId(), std::move(venus));
    // Earth
    auto earth = mnlt::GameObject::makePointLight(10.f);
    earth.transform.scale = glm::vec3{0.1f}; // Adjust scale
    earth.transform.translation = {9.0f, 0.0f, 0.0f}; // Position relative to the sun
    earth.color = {0.0f, 0.6f, 1.0f}; // Blue color for Earth
    earth.rigidBody.velocity = {.0f, .0f, 3.0f}; // Adjust initial velocity
    earth.rigidBody.mass = 1.0f; // Mass of Earth
    gameObjects.emplace(earth.getId(), std::move(earth));
    // Mars
    auto mars = mnlt::GameObject::makePointLight(10.f);
    mars.transform.scale = glm::vec3{0.08f}; // Adjust scale
    mars.transform.translation = {12.0f, 0.0f, 0.0f}; // Position relative to the sun
    mars.color = {1.0f, 0.3f, 0.0f}; // Reddish color for Mars
    mars.rigidBody.velocity = {.0f, .0f, 2.5f}; // Adjust initial velocity
    mars.rigidBody.mass = 0.107f; // Adjust mass relative to Earth
    gameObjects.emplace(mars.getId(), std::move(mars));
}
void TestApp::loadGameObjects() 
{
    std::shared_ptr<mnlt::Model> model = mnlt::Model::createModelFromFile(device, "assets/models/colored_cube.obj");
    auto cube = mnlt::GameObject::createGameObject();
    cube.model = model;
    cube.transform.translation = {-0.5f, -0.5f, 0.0f};
    cube.transform.scale = {0.2f, 0.2f, 0.2f};
    cube.name = "cube";
    gameObjects.emplace(cube.getId(), std::move(cube));
    model = mnlt::Model::createModelFromFile(device, "assets/models/smooth_vase.obj");
    auto smoothVase = mnlt::GameObject::createGameObject();
    smoothVase.model = model;
    smoothVase.transform.translation = {0.5f, 0.0f, 0.0f};
    smoothVase.transform.scale = {3.0f, 2.5f, 3.0f};
    smoothVase.name = "smoothvase";
    gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));
    model = mnlt::Model::createModelFromFile(device, "assets/models/quad.obj");
    auto floor = mnlt::GameObject::createGameObject();
    floor.model = model;
    floor.transform.translation = {0.0f, 0.0f, 0.0f};
    floor.transform.scale = {3.0f, 1.0f, 3.0f};
    floor.name = "floor";
    gameObjects.emplace(floor.getId(), std::move(floor));
    model = mnlt::Model::createModelFromFile(device, "assets/models/viking_room.obj");
    auto viking = mnlt::GameObject::createGameObject();
    viking.model = model;
    viking.transform.translation = {-5.f, 0.0f, 0.0f};
    viking.transform.scale = {1.0f, 1.0f, 1.0f};
    viking.name = "viking";
    gameObjects.emplace(viking.getId(), std::move(viking));
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
        auto pointLight = mnlt::GameObject::makePointLight(0.2f);
        pointLight.color = lightColors[i];
        auto rotateLight = glm::rotate(
            glm::mat4(1.f),
            (i * glm::two_pi<float>()) / lightColors.size(),
            {0.f, -1.f, 0.f});
        pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));
    }
}