#include "ui.hpp"

#include "device.hpp"
#include "game_object.hpp"
#include "window.hpp"

#include "../../libs/imgui/imgui_impl_glfw.h"
#include "../../libs/imgui/imgui_impl_vulkan.h"

namespace mnlt
{
    // ok this just initializes imgui using the provided integration files. So in our case we need to
    // initialize the vulkan and glfw imgui implementations, since that's what our engine is built
    // using.
    UI::UI(Window &window, Device &device, VkRenderPass renderPass, uint32_t imageCount, VkDescriptorPool descriptorPool) : device{device} 
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsClassic();

        // Setup Platform/Renderer backends
        // Initialize imgui for vulkan
        ImGui_ImplGlfw_InitForVulkan(window.getGLFWwindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = device.getInstance();
        init_info.PhysicalDevice = device.getPhysicalDevice();
        init_info.Device = device.device();
        init_info.QueueFamily = device.getGraphicsQueueFamily();
        init_info.Queue = device.graphicsQueue();

        // pipeline cache is a potential future optimization, ignoring for now
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = descriptorPool;
        // todo, I should probably get around to integrating a memory allocator library such as Vulkan
        // memory allocator (VMA) sooner than later. We don't want to have to update adding an allocator
        // in a ton of locations.
        init_info.Allocator = VK_NULL_HANDLE;
        init_info.MinImageCount = 2;
        init_info.ImageCount = imageCount;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info, renderPass);

        // upload fonts, this is done by recording and submitting a one time use command buffer
        // which can be done easily bye using some existing helper functions on the lve device object
        auto commandBuffer = device.beginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture();
        device.endSingleTimeCommands(commandBuffer);
        ImGui_ImplVulkan_DestroyFontsTexture();
    }
    UI::~UI() 
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    
    void UI::newFrame() 
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    // this tells imgui that we're done setting up the current frame,
    // then gets the draw data from imgui and uses it to record to the provided
    // command buffer the necessary draw commands
    void UI::render(VkCommandBuffer commandBuffer) 
    {
        ImGui::Render();
        ImDrawData *drawdata = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(drawdata, commandBuffer);
    }
    void UI::showGameObjectWindow(GameObject *gameObject)
    {
        if(gameObject->ui.showPropertyWindow)
        {
            if(ImGui::Begin(gameObject->name.c_str(), &gameObject->ui.showPropertyWindow))
            {
                ImGui::Text("Transform Component");
                ImGui::DragFloat3("Translation", &gameObject->transform.translation[0]);
                ImGui::DragFloat3("Rotation", &gameObject->transform.rotation[0]);
                ImGui::DragFloat3("Scale", &gameObject->transform.scale[0]);

                ImGui::Text("Rigidbody Component");
                ImGui::DragFloat3("Velocity", &gameObject->rigidBody.velocity[0]);
                ImGui::DragFloat("Mass", &gameObject->rigidBody.mass);

                ImGui::Text("Color Component");
                ImGui::ColorEdit3("Color", &gameObject->color[0]);

                if(gameObject->pointLight != nullptr)
                {
                    ImGui::Text("Pointlight Component");
                    ImGui::DragFloat("Intensity", &gameObject->pointLight->lightIntensity);
                }
            }
            ImGui::End();
        }
    }
    void UI::runExample(FrameInfo frameInfo) 
    {
        if (show_debug_window) ImGui::ShowMetricsWindow(&show_debug_window);

        ImGui::Begin("UI");  // Create a window called "Hello, world!" and append into it.
        ImGui::Checkbox("Debug Window", &show_debug_window);  // Edit bools storing our window open/close state
        ImGui::Text("Time:");
        ImGui::Text("DeltaTime: %f" ,frameInfo.time.getDeltaTime());
        ImGui::Text("PureDeltaTime: %f" ,frameInfo.time.getPureDeltaTime());
        ImGui::Text("FixedDeltaTime: %f" ,frameInfo.time.getFixedDeltaTime());
        float scale = 0;
        ImGui::InputDouble("Time Scale", &frameInfo.time.timeScale);
        ImGui::Text("Camera:");
        ImGui::Checkbox("Enable Grid", &frameInfo.camera.enableGrid);
        ImGui::DragInt("Intensity", &frameInfo.camera.gridSize);
        ImGui::Text("GameObjects:");
        for (auto& obj : frameInfo.gameObjects)
        {
            if(ImGui::Button(obj.second.name.c_str()))
            {
                obj.second.ui.showPropertyWindow = true;
            }
            showGameObjectWindow(&obj.second);
        }

        ImGui::End();
    }
}