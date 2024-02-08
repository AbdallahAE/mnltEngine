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
        loadGameObjects();
    }
    App::~App()
    {
        
    }

    void App::run()
    {
        SimpleRenderSystem simpleRenderSystem{device, renderer.getSwapChainRenderPass()};
        Camera camera{};

        auto viewerObject = GameObject::createGameObject();
        CameraController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!window.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(window.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
            
            if (auto commandBuffer = renderer.beginFrame()) 
            {
                renderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
                renderer.endSwapChainRenderPass(commandBuffer);
                renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(device.device());
    }


    void App::loadGameObjects() 
    {
        std::shared_ptr<Model> model = Model::createModelFromFile(device, "models/colored_cube.obj");;
        auto cube = GameObject::createGameObject();
        cube.model = model;
        cube.transform.translation = {.0f, .0f, 2.5f};
        cube.transform.scale = glm::vec3(3.f);
        gameObjects.push_back(std::move(cube));
    }
}