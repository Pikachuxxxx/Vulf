#include <iostream>

#include <VulfBase.h>


// Load the Instance extensions/layers and device Extensions
std::vector<const char*> g_ValidationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

std::vector<const char*> g_InstanceExtensions = {
    "VK_KHR_get_physical_device_properties2",
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

std::vector<const char*> g_DeviceExtensions = {
   VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if (__APPLE__)
   "VK_KHR_portability_subset"
#endif
};

using namespace Vulf;

class VulfHelloTriangle : public Vulf::VulfBase
{
public:
    VulfHelloTriangle() : VulfBase("Hello Triangle") {}

     ~VulfHelloTriangle() {
        VK_LOG("Quitting...");
        defaultVertShader.DestroyModule();
        defaultFragShader.DestroyModule();
    }

// Types
private:
    struct ModelPushConstant{
        alignas(16) glm::mat4 model;
    }modelPCData;

    struct ViewProjectionUBOData
    {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::mat4 _padding1 = glm::mat4(0.0f);
        alignas(16) glm::mat4 _padding2 = glm::mat4(0.0f);
    }vpUBOData;

    float someNum = 45.0f;
private:
    // default stuff required for initialization, these resources are all explicitly allocated and to not follow RAII, hence the defauly ones are provided by Vulf
    FixedPipelineFuncs      fixedFunctions;

    GraphicsPipeline        simpleGraphicsPipeline;

    VkPushConstantRange     modelPushConstant;

    DepthImage              depthImage;

    Framebuffer             simpleFrameBuffer;

    using ShaderStage = std::vector<VkPipelineShaderStageCreateInfo>;
    // Shaders
    Shader                  defaultVertShader;
    Shader                  defaultFragShader;
    ShaderStage             defaultShaders;

    // Buffers
    UniformBuffer           helloTriangleUBO;
    VertexBuffer            helloTriangleVBO;

    // Textures
    Texture                 gridTexture;
    Texture                 checkerTexture;

private:
    void LoadShaders() override {

        // Default shaders
        defaultVertShader.CreateShader((SHADER_BINARY_DIR) + std::string("/defaultVert.spv"), ShaderType::VERTEX_SHADER);
        defaultFragShader.CreateShader((SHADER_BINARY_DIR) + std::string("/defaultFrag.spv"), ShaderType::FRAGMENT_SHADER);
        defaultShaders.push_back(defaultVertShader.GetShaderStageInfo());
        defaultShaders.push_back(defaultFragShader.GetShaderStageInfo());
    }

    void BuildTextureResources() override {
        // default
        depthImage.CreateDepthImage(baseSwapchain.get_extent().width, baseSwapchain.get_extent().height, baseCommandPool);

        // Grid Texture
        gridTexture.CreateTexture((SRC_DIR) + std::string("/data/textures/TestGrid_1024.png"), baseCommandPool);

        // Checker Texture;
        checkerTexture.CreateTexture((SRC_DIR) + std::string("/data/textures/TestCheckerMap.png"), baseCommandPool);
    }

    void BuildBufferResource() override {
        // Triangle vertices and indices
        helloTriangleVBO.Create(rainbowTriangleVertices, baseCommandPool);

        // View Projection Uniform Buffer
        helloTriangleUBO.AddDescriptor(UniformBuffer::DescriptorInfo(0, ShaderType::VERTEX_SHADER, sizeof(ViewProjectionUBOData), 0));
        helloTriangleUBO.AddDescriptor(UniformBuffer::DescriptorInfo(1, ShaderType::FRAGMENT_SHADER, gridTexture));
        helloTriangleUBO.AddDescriptor(UniformBuffer::DescriptorInfo(2, ShaderType::FRAGMENT_SHADER, checkerTexture));
        helloTriangleUBO.CreateUniformBuffer(3, sizeof(ViewProjectionUBOData));
    }

    void BuildFixedPipeline() override {
        // Create the push constants
        modelPushConstant.stageFlags    = ShaderType::VERTEX_SHADER | ShaderType::FRAGMENT_SHADER;
        modelPushConstant.offset        = 0;
        modelPushConstant.size          = sizeof(ModelPushConstant);

        fixedFunctions.SetFixedPipelineStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, baseSwapchain.get_extent(), false);
        fixedFunctions.SetPipelineLayout(helloTriangleUBO.GetDescriptorSetLayout(), &modelPushConstant);
    }

    void BuildGraphicsPipeline() override {
        simpleGraphicsPipeline.Create(defaultShaders, fixedFunctions, baseRenderPass.GetRenderPass());
    }

    // default
    void BuildFramebuffer() override {
        simpleFrameBuffer.Create(baseRenderPass.GetRenderPass(), baseSwapchain.get_image_views(), depthImage.GetDepthImageView(), baseSwapchain.get_extent());
    }

    void CleanUpPipeline() override {
        ZoneScopedC(0xffffff);
        vkDeviceWaitIdle(VKDEVICE);
        simpleFrameBuffer.Destroy();
        gridTexture.Destroy();
        checkerTexture.Destroy();
        depthImage.Destroy();
        helloTriangleUBO.Destroy();
        helloTriangleVBO.Destroy();
        simpleGraphicsPipeline.Destroy();
        fixedFunctions.DestroyPipelineLayout();
    }


private:

    void OnStart() override
    {

    }

    void OnRender(CmdBuffer dcb) override
    {
        ZoneScopedC(0xffa500);
        OPTICK_EVENT();

        baseRenderPass.SetClearColor(0.0f, 0.0f, 0.0f);
        auto framebuffers = simpleFrameBuffer.GetFramebuffers();
        auto descriptorSets = helloTriangleUBO.GetSets();

#ifdef _WIN32
        OPTICK_GPU_CONTEXT(dcb.get_handle());
        OPTICK_GPU_EVENT("Recording cmd buffers");
#endif
        dcb.begin_recording();
        baseRenderPass.BeginRenderPass(dcb.get_handle(), framebuffers[get_image_idx()], baseSwapchain.get_extent());

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(getWindow()->getWidth());
        viewport.height = static_cast<float>(getWindow()->getHeight());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = { getWindow()->getWidth(), getWindow()->getHeight()};

        vkCmdSetViewport(dcb.get_handle(), 0, 1, &viewport);
        vkCmdSetScissor(dcb.get_handle(), 0, 1, &scissor);

        simpleGraphicsPipeline.Bind(dcb.get_handle());

        // Bind the appropriate descriptor sets
        vkCmdBindDescriptorSets(dcb.get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, fixedFunctions.GetPipelineLayout(), 0, 1, &descriptorSets[get_frame_idx()], 0, nullptr);

        // Bind the push constants
        modelPCData.model = glm::rotate(glm::mat4(1.0f), (float) glm::radians(90.0f * 1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkCmdPushConstants(dcb.get_handle(), fixedFunctions.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ModelPushConstant), &modelPCData);

        helloTriangleVBO.Bind(dcb.get_handle());

        vkCmdDraw(dcb.get_handle(), rainbowTriangleVertices.size(), 1, 0, 0);

        ImGuiIO& io = ImGui::GetIO();

        io.DisplaySize = ImVec2((float) getWindow()->getWidth(), (float) getWindow()->getHeight());

        get_ui_overlay().update_imgui_buffers();
            get_ui_overlay().draw(dcb.get_handle());

        baseRenderPass.EndRenderPass(dcb.get_handle());
        dcb.end_recording();
    }

    void OnUpdateBuffers(uint32_t frameIdx) override {
        vpUBOData.view = glm::mat4(1.0f);
        vpUBOData.proj = glm::mat4(1.0f);

        //vpUBOData.view = getCamera().GetViewMatrix();
        //vpUBOData.proj = glm::perspective(glm::radians(someNum), (float) baseSwapchain.get_extent().width / baseSwapchain.get_extent().height, 0.01f, 100.0f);
        ////vpUBOData.proj[1][1] *= -1;

        helloTriangleUBO.UpdateBuffer(&vpUBOData, sizeof(ViewProjectionUBOData), frameIdx);
    }

    void OnImGui() override
    {
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        if(ImGui::Begin("Vulkan Example"))
        {
            ImGui::Text("Hello ImGui");
            ImGui::TextUnformatted(get_app_name().c_str());
            ImGui::DragFloat("Position", &someNum, 0.1f, 0.0f, 100.0f);

            ImGui::Image((void*)gridTexture.get_descriptor_set(), ImVec2(ImGui::GetWindowSize()[0], 200), ImVec2(0, 0), ImVec2(1.0f, -1.0f));
            ImGui::Image((void*)checkerTexture.get_descriptor_set(), ImVec2(ImGui::GetWindowSize()[0], 200), ImVec2(0, 0), ImVec2(1.0f, -1.0f));
        }
        ImGui::End();

        ImGui::Render();
    }
};

VULF_MAIN(HelloTriangle)
