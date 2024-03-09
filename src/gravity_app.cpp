#include "gravity_app.hpp"


void GravityApp::start()
{
    ubo.ambientLightColor = glm::vec4(1.f);
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
    auto& sun = gameObjectManager.createGameObject();
    sun.name = "Sun";
    sun.transform.scale = glm::vec3{1.f}; // Increase the size for better visualization
    sun.transform.translation = {0.0f, 0.0f, 0.0f};
    sun.model = mnlt::Model::createModelFromFile(device, "assets/models/sphere.obj");
    sun.diffuseMap = mnlt::Texture::createTextureFromFile(device, {"assets/textures/sun.jpg"});
    sun.rigidBody.velocity = {.0f, .0f, .0f};
    sun.rigidBody.mass = 1.989e24f; // Sun's mass is significantly higher
    // Create Planets
    // Mercury
    auto& mercury = gameObjectManager.createGameObject();
    mercury.name = "Mercury";
    mercury.transform.scale = glm::vec3{1.0f}; // Adjust scale
    mercury.transform.translation = {57.9e-1f, 0.0f, 0.0f}; // Position relative to the sun
    mercury.model = mnlt::Model::createModelFromFile(device, "assets/models/sphere.obj");
    mercury.diffuseMap = mnlt::Texture::createTextureFromFile(device, {"assets/textures/mercury.jpg"});
    mercury.rigidBody.mass = 3.285e16f; // Adjust mass relative to Earth
    mercury.rigidBody.velocity = {.0f, .0f, (glm::sqrt(gravitySystem.strengthGravity * sun.rigidBody.mass / (mercury.transform.translation.x - sun.transform.translation.x)))}; // Adjust initial velocity
    // Venus
    auto& venus = gameObjectManager.createGameObject();
    venus.name = "Venus";
    venus.transform.scale = glm::vec3{1.0f}; // Adjust scale
    venus.transform.translation = {108.2e-1f, 0.0f, 0.0f}; // Position relative to the sun
    venus.model = mnlt::Model::createModelFromFile(device, "assets/models/sphere.obj");
    venus.diffuseMap = mnlt::Texture::createTextureFromFile(device, {"assets/textures/venus.jpg"});
    venus.rigidBody.mass = 4.867e17f; // Adjust mass relative to Earth
    venus.rigidBody.velocity = {.0f, .0f, (glm::sqrt(gravitySystem.strengthGravity * sun.rigidBody.mass / (venus.transform.translation.x - sun.transform.translation.x)))}; // Adjust initial velocity
    // Earth
    auto& earth = gameObjectManager.createGameObject();
    earth.name = "Earth";
    earth.transform.scale = glm::vec3{1.0f}; // Adjust scale
    earth.transform.translation = {149.6e-1f, 0.0f, 0.0f}; // Position relative to the sun
    earth.model = mnlt::Model::createModelFromFile(device, "assets/models/sphere.obj");
    earth.diffuseMap = mnlt::Texture::createTextureFromFile(device, {"assets/textures/earth.jpg"});
    earth.rigidBody.mass = 5.972e17f; // Mass of Earth
    earth.rigidBody.velocity = {.0f, .0f, (glm::sqrt(gravitySystem.strengthGravity * sun.rigidBody.mass / (earth.transform.translation.x - sun.transform.translation.x)))}; // Adjust initial velocity
    // Mars
    auto& mars = gameObjectManager.createGameObject();
    mars.name = "Mars";
    mars.transform.scale = glm::vec3{1.0f}; // Adjust scale
    mars.transform.translation = {227.9e-1f, 0.0f, 0.0f}; // Position relative to the sun
    mars.model = mnlt::Model::createModelFromFile(device, "assets/models/sphere.obj");
    mars.diffuseMap = mnlt::Texture::createTextureFromFile(device, {"assets/textures/mars.jpg"});
    mars.rigidBody.mass = 6.39e16f; // Adjust mass relative to Earth
    mars.rigidBody.velocity = {.0f, .0f, (glm::sqrt(gravitySystem.strengthGravity * sun.rigidBody.mass / (mars.transform.translation.x - sun.transform.translation.x)))}; // Adjust initial velocity
    // jupitar
    auto& jupitar = gameObjectManager.createGameObject();
    jupitar.name = "Jupitar";
    jupitar.transform.scale = glm::vec3{1.0f}; // Adjust scale
    jupitar.transform.translation = {778.6e-1f, 0.0f, 0.0f}; // Position relative to the sun
    jupitar.model = mnlt::Model::createModelFromFile(device, "assets/models/sphere.obj");
    jupitar.diffuseMap = mnlt::Texture::createTextureFromFile(device, {"assets/textures/jupitar.jpg"});
    jupitar.rigidBody.mass = 1.898e20f; // Adjust mass relative to Earth
    jupitar.rigidBody.velocity = {.0f, .0f, (glm::sqrt(gravitySystem.strengthGravity * sun.rigidBody.mass / (jupitar.transform.translation.x - sun.transform.translation.x)))}; // Adjust initial velocity
    // Saturn
    auto& saturn = gameObjectManager.createGameObject();
    saturn.name = "Saturn";
    saturn.transform.scale = glm::vec3{1.0f}; // Adjust scale
    saturn.transform.translation = {1433.5e-1f, 0.0f, 0.0f}; // Position relative to the sun
    saturn.model = mnlt::Model::createModelFromFile(device, "assets/models/sphere.obj");
    saturn.diffuseMap = mnlt::Texture::createTextureFromFile(device, {"assets/textures/saturn.jpg"});
    saturn.rigidBody.mass = 5.683e19f; // Adjust mass relative to Earth
    saturn.rigidBody.velocity = {.0f, .0f, (glm::sqrt(gravitySystem.strengthGravity * sun.rigidBody.mass / (saturn.transform.translation.x - sun.transform.translation.x)))}; // Adjust initial velocity
    // Uranus
    auto& uranus = gameObjectManager.createGameObject();
    uranus.name = "Uranus";
    uranus.transform.scale = glm::vec3{1.0f}; // Adjust scale
    uranus.transform.translation = {2872.5e-1f, 0.0f, 0.0f}; // Position relative to the sun
    uranus.model = mnlt::Model::createModelFromFile(device, "assets/models/sphere.obj");
    uranus.diffuseMap = mnlt::Texture::createTextureFromFile(device, {"assets/textures/missing.png"});
    uranus.rigidBody.mass = 8.681e18f; // Adjust mass relative to Earth
    uranus.rigidBody.velocity = {.0f, .0f, (glm::sqrt(gravitySystem.strengthGravity * sun.rigidBody.mass / (uranus.transform.translation.x - sun.transform.translation.x)))}; // Adjust initial velocity
    // Neptune
    auto& Neptune = gameObjectManager.createGameObject();
    Neptune.name = "Neptune";
    Neptune.transform.scale = glm::vec3{1.0f}; // Adjust scale
    Neptune.transform.translation = {4495.1e-1f, 0.0f, 0.0f}; // Position relative to the sun
    Neptune.model = mnlt::Model::createModelFromFile(device, "assets/models/sphere.obj");
    Neptune.diffuseMap = mnlt::Texture::createTextureFromFile(device, {"assets/textures/neptune.jpg"});
    Neptune.rigidBody.mass = 1.024e19f; // Adjust mass relative to Earth
    Neptune.rigidBody.velocity = {.0f, .0f, (glm::sqrt(gravitySystem.strengthGravity * sun.rigidBody.mass / (Neptune.transform.translation.x - sun.transform.translation.x)))}; // Adjust initial velocity 
}
