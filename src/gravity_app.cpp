#include "gravity_app.hpp"


void GravityApp::start()
{
    camera.setPerspectiveProjection(glm::radians(50.f), renderer.getAspectRatio(), 0.1f, 1000.f);

    ui.initialize(renderer.getSwapChainRenderPass(), renderer.getImageCount(), globalPool->getDescriptorPool());
    simpleRenderSystem.createRenderer(renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
    pointLightSystem.createRenderer(renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
    gridSystem.createRenderer(renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());

    loadPhysicsObjects();
}
void GravityApp::renderSystems(VkCommandBuffer commandBuffer, mnlt::FrameInfo frameInfo)
{
    simpleRenderSystem.renderGameObjects(frameInfo);
    pointLightSystem.renderLights(frameInfo, ubo);
    if(camera.enableGrid)
        gridSystem.render(frameInfo);
    ui.newFrame();
    ui.runExample(frameInfo);
    ui.render(commandBuffer);
}
void GravityApp::update(mnlt::Time time)
{
    if(window.wasWindowResized())
        camera.setPerspectiveProjection(glm::radians(50.f), renderer.getAspectRatio(), 0.1f, 1000.f);
    camera.move(window.getGLFWwindow(), time.getPureDeltaTime());
    
    gravitySystem.update(&gameObjectManager.gameObjects, time, 100);
}

void GravityPhysicsSystem::update(mnlt::GameObject::Map* physicsObjs, mnlt::Time time, int substeps) 
{
    for(int i=0; i<substeps; i++)
    stepSimulation(physicsObjs, time.getDeltaTime() / substeps);
}
glm::vec3 GravityPhysicsSystem::computeForce(mnlt::GameObject& fromObj, mnlt::GameObject& toObj) const {
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
void GravityPhysicsSystem::stepSimulation(mnlt::GameObject::Map* physicsObjs, float deltaTime) 
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


void GravityApp::loadPhysicsObjects()
{
    // Create the Sun
    auto& sun = gameObjectManager.makePointLight(10.f);
    sun.transform.scale = glm::vec3{0.2f}; // Increase the size for better visualization
    sun.transform.translation = {0.0f, 0.0f, 0.0f};
    sun.color = {1.f, 1.f, 0.f}; // Yellow color for the sun
    sun.rigidBody.velocity = {.0f, .0f, .0f};
    sun.rigidBody.mass = 333000.f; // Sun's mass is significantly higher
    // Create Planets
    // Mercury
    auto& mercury = gameObjectManager.makePointLight(10.f);
    mercury.transform.scale = glm::vec3{0.05f}; // Adjust scale
    mercury.transform.translation = {4.0f, 0.0f, 0.0f}; // Position relative to the sun
    mercury.color = {0.7f, 0.7f, 0.7f}; // Gray color for Mercury
    mercury.rigidBody.velocity = {.0f, .0f, 7.0f}; // Adjust initial velocity
    mercury.rigidBody.mass = 0.055f; // Adjust mass relative to Earth
    // Venus
    auto& venus = gameObjectManager.makePointLight(10.f);
    venus.transform.scale = glm::vec3{0.08f}; // Adjust scale
    venus.transform.translation = {6.0f, 0.0f, 0.0f}; // Position relative to the sun
    venus.color = {0.8f, 0.5f, 0.1f}; // Brownish color for Venus
    venus.rigidBody.velocity = {.0f, .0f, 5.0f}; // Adjust initial velocity
    venus.rigidBody.mass = 0.815f; // Adjust mass relative to Earth
    // Earth
    auto& earth = gameObjectManager.makePointLight(10.f);
    earth.transform.scale = glm::vec3{0.1f}; // Adjust scale
    earth.transform.translation = {9.0f, 0.0f, 0.0f}; // Position relative to the sun
    earth.color = {0.0f, 0.6f, 1.0f}; // Blue color for Earth
    earth.rigidBody.velocity = {.0f, .0f, 3.0f}; // Adjust initial velocity
    earth.rigidBody.mass = 1.0f; // Mass of Earth
    // Mars
    auto& mars = gameObjectManager.makePointLight(10.f);
    mars.transform.scale = glm::vec3{0.08f}; // Adjust scale
    mars.transform.translation = {12.0f, 0.0f, 0.0f}; // Position relative to the sun
    mars.color = {1.0f, 0.3f, 0.0f}; // Reddish color for Mars
    mars.rigidBody.velocity = {.0f, .0f, 2.5f}; // Adjust initial velocity
    mars.rigidBody.mass = 0.107f; // Adjust mass relative to Earth
}