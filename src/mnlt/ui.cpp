#include "ui.hpp"

#include "device.hpp"
#include "game_object.hpp"
#include "window.hpp"

#include "../../libs/imgui/imgui_impl_glfw.h"
#include "../../libs/imgui/imgui_impl_vulkan.h"
#include "../../libs/imgui/imgui.h"

namespace mnlt
{
    // ok this just initializes imgui using the provided integration files. So in our case we need to
    // initialize the vulkan and glfw imgui implementations, since that's what our engine is built
    // using.
    UI::UI(Window &window, Device &device) : window{window}, device{device}
    {
        
    }
    UI::~UI() 
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    
    void UI::initialize(VkRenderPass renderPass, uint32_t imageCount, VkDescriptorPool descriptorPool)
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(window.getGLFWwindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = device.getInstance();
        init_info.PhysicalDevice = device.getPhysicalDevice();
        init_info.Device = device.device();
        init_info.QueueFamily = device.getGraphicsQueueFamily();
        init_info.Queue = device.graphicsQueue();
        //init_info.PipelineRenderingCreateInfo = ;
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = descriptorPool;
        init_info.Allocator = VK_NULL_HANDLE;
        init_info.MinImageCount = 2;
        init_info.ImageCount = imageCount;
        init_info.CheckVkResultFn = check_vk_result;
        init_info.RenderPass = renderPass;
        //init_info.Subpass = ;
        //init_info.UseDynamicRendering = ;
        //init_info.MinAllocationSize = ;
        //init_info.MSAASamples = ;

        ImGui_ImplVulkan_Init(&init_info);

        auto commandBuffer = device.beginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture();
        device.endSingleTimeCommands(commandBuffer);
        ImGui_ImplVulkan_DestroyFontsTexture();
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
        ImGui::Begin("Properties");
        if(gameObject->ui.showPropertyWindow)
        {
            ImGui::Text("%s", gameObject->name.c_str());
            ImGui::Text("Transform Component");
            ImGui::DragFloat3("Translation", &gameObject->transform.translation[0], 0.1f);
            ImGui::DragFloat3("Rotation", &gameObject->transform.rotation[0], 0.1f);
            ImGui::DragFloat3("Scale", &gameObject->transform.scale[0], 0.1f);
            ImGui::Text("Rigidbody Component");
            ImGui::DragFloat3("Velocity", &gameObject->rigidBody.velocity[0], 0.1f);
            ImGui::DragFloat("Mass", &gameObject->rigidBody.mass, 0.1f);
            ImGui::Text("Color Component");
            ImGui::ColorEdit3("Color", &gameObject->color[0]);
            if(gameObject->pointLight != nullptr)
            {
                ImGui::Text("Pointlight Component");
                ImGui::DragFloat("Intensity", &gameObject->pointLight->lightIntensity, 0.1f);
            }
        }
        ImGui::End();
    }
    void UI::runExample(FrameInfo frameInfo) 
    {
        // Set up a full-screen window for docking
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoBringToFrontOnFocus |  
            ImGuiWindowFlags_MenuBar |
            ImGuiWindowFlags_NoNavFocus |                                                      
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBackground;

        // Begin DockSpace
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("UI", nullptr, windowFlags);
        ImGui::PopStyleVar(3);
        ImGui::DockSpace(ImGui::GetID("Dockspace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        {
            if (ImGui::BeginMenuBar())
            {
                // Debug Tab
                if (ImGui::BeginMenu("Debug"))
                {
                    ImGui::Checkbox("Debug Window", &show_debug_window);
                    if(show_debug_window) {ImGui::ShowMetricsWindow(&show_debug_window);}
                    ImGui::EndMenu();
                }
                // Time Tab
                if (ImGui::BeginMenu("Time"))
                {
                    ImGui::Text("DeltaTime: %f", frameInfo.time.getDeltaTime());
                    ImGui::Text("PureDeltaTime: %f", frameInfo.time.getPureDeltaTime());
                    ImGui::Text("FixedDeltaTime: %f", frameInfo.time.getFixedDeltaTime());
                    ImGui::InputDouble("Time Scale", &frameInfo.time.timeScale);
                    ImGui::EndMenu();
                }
                // Camera Tab
                if (ImGui::BeginMenu("Camera"))
                {
                    ImGui::Checkbox("Enable Grid", &frameInfo.camera.enableGrid);
                    ImGui::DragInt("Intensity", &frameInfo.camera.gridSize);
                    ImGui::DragFloat3("Translation", &frameInfo.camera.viewerObject.translation[0], 0.1f);
                    ImGui::DragFloat3("Rotation", &frameInfo.camera.viewerObject.rotation[0], 0.1f);
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            
            static std::string selectedGameObjectName;
            ImGui::Begin("Scene");
            ImGui::Text("GameObjects:");
            for (auto& obj : frameInfo.gameObjects) {
                // Highlight the selectable if it's the selected GameObject
                bool isSelected = (obj.second.name == selectedGameObjectName);

                if (ImGui::Selectable(obj.second.name.c_str(), isSelected)) {
                    // Update the selected GameObject name
                    selectedGameObjectName = obj.second.name;
                }
                
                // Open the property window for the selected GameObject
                if (isSelected) {
                    obj.second.ui.showPropertyWindow = true;
                    showGameObjectWindow(&obj.second);
                } else {
                    obj.second.ui.showPropertyWindow = false;
                }
            }
            ImGui::End();
        }

        // End DockSpace
        ImGui::End(); 
    }
}