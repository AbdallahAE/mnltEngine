#include "app.hpp"
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

        // build frame descriptor pools
        framePools.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        auto framePoolBuilder = DescriptorPool::Builder(device)
                                    .setMaxSets(1000)
                                    .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                                    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
                                    .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
        for (int i = 0; i < framePools.size(); i++) {
            framePools[i] = framePoolBuilder.build();
        }
      
    }
    App::~App()
    {
        
    }

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

        globalSetLayout = DescriptorSetLayout::Builder(device)
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

        start();

        Time time;

        while (!window.shouldClose())
        {
            glfwPollEvents();

            time.update();
            
            if (auto commandBuffer = renderer.beginFrame()) 
            {
                int frameIndex = renderer.getFrameIndex();
                framePools[frameIndex]->resetPool();
                FrameInfo frameInfo{frameIndex, time, commandBuffer, camera, globalDescriptorSets[frameIndex], *framePools[frameIndex], gameObjectManager.gameObjects};

                update(time);
                // final step of update is updating the game objects buffer data
                // The render functions MUST not change a game objects transform data
                gameObjectManager.updateBuffer(frameIndex);
        
                renderer.beginSwapChainRenderPass(commandBuffer);
                renderSystems(commandBuffer, frameInfo);

                // update
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                renderer.endSwapChainRenderPass(commandBuffer);
                renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(device.device());
    }
}