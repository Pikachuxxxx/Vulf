#include <iostream>

#include <VulfBase.h>


// Load the Instance extensions/layers and device Extensions
std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

std::vector<const char*> instanceExtensions = {
    "VK_KHR_get_physical_device_properties2",
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

std::vector<const char*> deviceExtensions = {
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
        CleanUpPipeline();
        _def_CommandPool.Destroy();
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

    CmdBuffer               simpleCommandBuffer;

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
        depthImage.CreateDepthImage(_def_Swapchain.GetSwapExtent().width, _def_Swapchain.GetSwapExtent().height, _def_CommandPool);

        // Grid Texture
        gridTexture.CreateTexture((SRC_DIR) + std::string("/data/textures/TestGrid_256.png"), _def_CommandPool);

        // Checker Texture;
        checkerTexture.CreateTexture((SRC_DIR) + std::string("/data/textures/TestCheckerMap.png"), _def_CommandPool);
    }

    void BuildBufferResource() override {
        // Triangle vertices and indices
        helloTriangleVBO.Create(rainbowTriangleVertices, _def_CommandPool);

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

        fixedFunctions.SetFixedPipelineStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, _def_Swapchain.GetSwapExtent(), false);
        fixedFunctions.SetPipelineLayout(helloTriangleUBO.GetDescriptorSetLayout(), modelPushConstant);
    }

    void BuildGraphicsPipeline() override {
        simpleGraphicsPipeline.Create(defaultShaders, fixedFunctions, _def_RenderPass.GetRenderPass());
    }

    // default
    void BuildFramebuffer() override {
        simpleFrameBuffer.Create(_def_RenderPass.GetRenderPass(), _def_Swapchain.GetSwapImageViews(), depthImage.GetDepthImageView(), _def_Swapchain.GetSwapExtent());
    }

    // default
    void BuildCommandBuffers() override {
        simpleCommandBuffer.AllocateBuffers(_def_CommandPool.GetPool());

    }

    void UpdateBuffers(uint32_t imageIndex) override {
        vpUBOData.view = glm::mat4(1.0f);
        vpUBOData.proj = glm::mat4(1.0f);

        vpUBOData.view = getCamera().GetViewMatrix();
        vpUBOData.proj = glm::perspective(glm::radians(someNum), (float) _def_Swapchain.GetSwapExtent().width / _def_Swapchain.GetSwapExtent().height, 0.01f, 100.0f);
        //vpUBOData.proj[1][1] *= -1;

        helloTriangleUBO.UpdateBuffer(&vpUBOData, sizeof(ViewProjectionUBOData), imageIndex);
    }

    void CleanUpPipeline() override {
        ZoneScopedC(0xffffff);

        simpleCommandBuffer.Destroy(_def_CommandPool.GetPool());
        simpleFrameBuffer.Destroy();
        gridTexture.Destroy();
        checkerTexture.Destroy();
        depthImage.Destroy();
        helloTriangleUBO.Destroy();
        helloTriangleVBO.Destroy();
        simpleGraphicsPipeline.Destroy();
        _def_RenderPass.Destroy();
        fixedFunctions.DestroyPipelineLayout();
        _def_Swapchain.Destroy();
    }


private:

    void OnStart() override
    {
        /*
        _def_RenderPass.SetClearColor(0.0f, 0.0f, 0.0f);
        auto& cmdBuffers = simpleCommandBuffer.GetBuffers();
        auto framebuffers = simpleFrameBuffer.GetFramebuffers();
        auto descriptorSets = helloTriangleUBO.GetSets();

        for (int i = 0; i < cmdBuffers.size(); i++) {

#ifdef _WIN32
            OPTICK_GPU_CONTEXT(cmdBuffers[i]);
            OPTICK_GPU_EVENT("Recording cmd buffers");
#endif
            simpleCommandBuffer.RecordBuffer(cmdBuffers[i]);
            _def_RenderPass.BeginRenderPass(cmdBuffers[i], framebuffers[i], _def_Swapchain.GetSwapExtent());

            simpleGraphicsPipeline.Bind(cmdBuffers[i]);

            // Bind the appropriate descriptor sets
            vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, fixedFunctions.GetPipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);

            // Bind the push constants
            modelPCData.model = glm::rotate(glm::mat4(1.0f), (float) glm::radians(90.0f * 2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            vkCmdPushConstants(cmdBuffers[i], fixedFunctions.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ModelPushConstant), &modelPCData);

            helloTriangleVBO.Bind(cmdBuffers[i]);

            vkCmdDraw(cmdBuffers[i], rainbowTriangleVertices.size(), 1, 0, 0);

            ImGuiIO& io = ImGui::GetIO();

            io.DisplaySize = ImVec2((float) width, (float) height);

            ImGui::NewFrame();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
            ImGui::SetNextWindowPos(ImVec2(10, 10));
            ImGui::Begin("Vulkan Example", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            ImGui::TextUnformatted(get_app_name().c_str());
            ImGui::TextUnformatted(get_logical_device().Get()->GetGPUManager().get_device_name().c_str());

            ImGui::End();
            ImGui::PopStyleVar();
            ImGui::Render();


            if (get_ui_overlay().update_imgui_buffers())
                get_ui_overlay().draw(cmdBuffers[i]);

            _def_RenderPass.EndRenderPass(cmdBuffers[i]);
            simpleCommandBuffer.EndRecordingBuffer(cmdBuffers[i]);
        }

        submissionCommandBuffers.clear();
        submissionCommandBuffers.push_back(simpleCommandBuffer);
        */
    }

    void OnRender() override
    {
        ZoneScopedC(0xffa500);
        OPTICK_EVENT();

        _def_RenderPass.SetClearColor(0.0f, 0.0f, 0.0f);
        auto& cmdBuffers = simpleCommandBuffer.GetBuffers();
        auto framebuffers = simpleFrameBuffer.GetFramebuffers();
        auto descriptorSets = helloTriangleUBO.GetSets();

        int i = get_next_image_index();


#ifdef _WIN32
        OPTICK_GPU_CONTEXT(cmdBuffers[i]);
        OPTICK_GPU_EVENT("Recording cmd buffers");
#endif
        simpleCommandBuffer.RecordBuffer(cmdBuffers[i]);
        _def_RenderPass.BeginRenderPass(cmdBuffers[i], framebuffers[i], _def_Swapchain.GetSwapExtent());

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = { getWindow()->getWidth(), getWindow()->getHeight()};

        vkCmdSetViewport(cmdBuffers[i], 0, 1, &viewport);
        vkCmdSetScissor(cmdBuffers[i], 0, 1, &scissor);

        simpleGraphicsPipeline.Bind(cmdBuffers[i]);

        // Bind the appropriate descriptor sets
        vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, fixedFunctions.GetPipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);

        // Bind the push constants
        modelPCData.model = glm::rotate(glm::mat4(1.0f), (float) glm::radians(90.0f * 1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkCmdPushConstants(cmdBuffers[i], fixedFunctions.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ModelPushConstant), &modelPCData);

        helloTriangleVBO.Bind(cmdBuffers[i]);

        vkCmdDraw(cmdBuffers[i], rainbowTriangleVertices.size(), 1, 0, 0);

        
        ImGuiIO& io = ImGui::GetIO();

        io.DisplaySize = ImVec2((float) width, (float) height);

        get_ui_overlay().update_imgui_buffers();
            get_ui_overlay().draw(cmdBuffers[i]);

        _def_RenderPass.EndRenderPass(cmdBuffers[i]);
        simpleCommandBuffer.EndRecordingBuffer(cmdBuffers[i]);

        submissionCommandBuffers.clear();
        submissionCommandBuffers.push_back(simpleCommandBuffer);
  
    }

    void OnImGui() override
    {
        ImGui::NewFrame();

        //ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
        //ImGui::SetNextWindowPos(ImVec2(10, 10));
        if(ImGui::Begin("Vulkan Example"))
        {
            ImGui::Text("Hello ImGui");
            ImGui::TextUnformatted(get_app_name().c_str());
            ImGui::DragFloat("Position", &someNum, 0.1f, 0.0f, 100.0f);

            get_ui_overlay().setImageSet(gridTexture.get_descriptor_set());
            ImGui::Image((ImTextureID)gridTexture.get_descriptor_set(), ImVec2(ImGui::GetWindowSize()[0], 200), ImVec2(0, 0), ImVec2(1.0f, -1.0f));
        }
        ImGui::End();

        ImGui::Render();
    }
};

VULF_MAIN(HelloTriangle)
