#include "test_app.hpp"
#include "mnlt/game_object.hpp"
#include "mnlt/texture.hpp"
#include <GLFW/glfw3.h>
#include <vector>


void TestApp::start()
{
    camera.setPerspectiveProjection(glm::radians(50.f), renderer.getAspectRatio(), 0.1f, 1000.f);

    /* ubo.ambientLightColor = glm::vec4(1.f); */

    ui.initialize(renderer.getSwapChainRenderPass(), renderer.getImageCount(), globalPool->getDescriptorPool());
    simpleRenderSystem.createRenderer(renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
    pointLightSystem.createRenderer(renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
    gridSystem.createRenderer(renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());

    loadGameObjects();
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
    camera.setPerspectiveProjection(glm::radians(50.f), renderer.getAspectRatio(), 0.1f, 1000.f);
    camera.move(window.getGLFWwindow(), time.getPureDeltaTime());
}

void TestApp::loadGameObjects() 
{
    std::shared_ptr<mnlt::Model> model = mnlt::Model::createModelFromFile(device, "assets/models/colored_cube.obj");
    auto& cube = gameObjectManager.createGameObject();
    cube.model = model;
    cube.transform.translation = {-1.5f, -0.5f, 0.0f};
    cube.transform.scale = {0.2f, 0.2f, 0.2f};
    cube.name = "cube";
    model = mnlt::Model::createModelFromFile(device, "assets/models/smooth_vase.obj");
    auto& smoothVase = gameObjectManager.createGameObject();
    smoothVase.model = model;
    smoothVase.transform.translation = {1.5f, 0.0f, 0.0f};
    smoothVase.transform.scale = {3.0f, 2.5f, 3.0f};
    smoothVase.name = "smoothvase";
    model = mnlt::Model::createModelFromFile(device, "assets/models/quad.obj");
    auto& floor = gameObjectManager.createGameObject();
    floor.model = model;
    floor.transform.translation = {0.0f, 0.0f, 0.0f};
    floor.transform.scale = {3.0f, 1.0f, 3.0f};
    floor.name = "floor";
    model = mnlt::Model::createModelFromFile(device, "assets/models/viking_room.obj");
    auto& viking = gameObjectManager.createGameObject();
    viking.model = model;
    viking.diffuseMap = mnlt::Texture::createTextureFromFile(device, {"assets/textures/viking_room.png"});
    viking.transform.translation = {0.0f, -.1f, 0.0f};
    viking.transform.rotation = {1.55f, 1.55f, 0.f};
    viking.transform.scale = {1.0f, 1.0f, 1.0f};
    viking.name = "viking";




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
        auto& pointLight = gameObjectManager.makePointLight(0.2f);
        pointLight.color = lightColors[i];
        auto rotateLight = glm::rotate(
            glm::mat4(1.f),
            (i * glm::two_pi<float>()) / lightColors.size(),
            {0.f, -1.f, 0.f});
        pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
    }
}
