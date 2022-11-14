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

class VulfOverDrawVis : public Vulf::VulfBase
{
public:
    VulfOverDrawVis() : VulfBase("Over Draw Vis Test")
    {
        bunnyMesh = LoadObjModel((SRC_DIR)+std::string("/data/models/sponza.obj"));
    }

    ~VulfOverDrawVis() {
        VK_LOG("Quitting...");
        defaultVertShader.Destroy();
        defaultFragShader.Destroy();
    }

    // Types
private:
    struct ModelPushConstant {
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
    float aspectRatio = 3840 / 2160;

private:
    // default stuff required for initialization, these resources are all explicitly allocated and to not follow RAII, hence the defauly ones are provided by Vulf
    FixedPipelineFuncs      geomFixedFunctions;
    FixedPipelineFuncs      postFixedFunctions;

    GraphicsPipeline        geomPassPipeline;
    GraphicsPipeline        postProcessPipeline;

    VkPushConstantRange     modelPushConstant;

    DepthImage              depthImage;

    Framebuffer             simpleFrameBuffer;

    using ShaderStages = std::vector<VkPipelineShaderStageCreateInfo>;
    // Shaders
    Shader                  defaultVertShader;
    Shader                  defaultFragShader;

    Shader                  quadVertShader;
    Shader                  quadFragImgShader;

    ShaderStages             subdivisionShaders;
    ShaderStages             quadShaders;

    // Buffers
    UniformBuffer           helloTriangleUBO;
    VertexBuffer            helloTriangleVBO;

    VertexBuffer            quadVB;
    IndexBuffer             quadIB;

    VertexBuffer            stanford_bunnyVB;
    // IndexBuffer             stanford_bunnyIB;


    StorageImage            storageImage;

    // Textures
    Texture                 gridTexture;
    Texture                 checkerTexture;

    std::vector<DescriptorSet>           set_per_frame;

    std::vector<DescriptorSet>           post_process_set_per_frame;

    Mesh                    bunnyMesh;

private:
    void LoadShaders() override {

        // Default shaders
        defaultVertShader.Init((SHADER_BINARY_DIR)+std::string("/default.vert.spv"), ShaderType::VERTEX_SHADER);
        defaultFragShader.Init((SHADER_BINARY_DIR)+std::string("/default.frag.spv"), ShaderType::FRAGMENT_SHADER);

        quadVertShader.Init((SHADER_BINARY_DIR)+std::string("/quad.vert.spv"), ShaderType::VERTEX_SHADER);
        quadFragImgShader.Init((SHADER_BINARY_DIR)+std::string("/overdrawvis.frag.spv"), ShaderType::FRAGMENT_SHADER);

        subdivisionShaders.push_back(defaultVertShader.get_shader_stage_info());
        subdivisionShaders.push_back(defaultFragShader.get_shader_stage_info());

        quadShaders.push_back(quadVertShader.get_shader_stage_info());
        quadShaders.push_back(quadFragImgShader.get_shader_stage_info());

    }

    void BuildTextureResources() override {
        // default
        depthImage.CreateDepthImage(baseSwapchain.get_extent().width, baseSwapchain.get_extent().height, graphicsCommandPool);

        // Grid Texture
        gridTexture.Init((SRC_DIR)+std::string("/data/textures/TestGrid_1024.png"));

        // Checker Texture;
        checkerTexture.Init((SRC_DIR)+std::string("/data/textures/TestCheckerMap.png"));
    }

    void BuildBufferResource() override {
        // Triangle vertices and indices
        helloTriangleVBO.Init(rainbowTriangleVertices);
        helloTriangleUBO.Init(sizeof(ViewProjectionUBOData));

        // Quad VB & IB
        quadVB.Init(whiteQuadVertices);
        quadIB.Init(whiteQuadIndices);

        VK_LOG("Bunny Mesh");
            VK_LOG("\t Vertices : ", bunnyMesh.vertices.size());
            VK_LOG("\t Parts : ", bunnyMesh.parts.size());
        stanford_bunnyVB.Init(bunnyMesh.vertices);

        storageImage.Init(getWindow()->getWidth(), getWindow()->getHeight());

        DescriptorInfo uboInfo(DescriptorType::UNIFORM_BUFFER, 0, ShaderType::VERTEX_SHADER);
        uboInfo.attach_resource<UniformBuffer>(&helloTriangleUBO);

        DescriptorInfo gridTexInfo(DescriptorType::COMBINED_IMAGE_SAMPLER, 1, ShaderType::FRAGMENT_SHADER);
        gridTexInfo.attach_resource<Texture>(&gridTexture);

        DescriptorInfo checkerTexInfo(DescriptorType::COMBINED_IMAGE_SAMPLER, 2, ShaderType::FRAGMENT_SHADER);
        checkerTexInfo.attach_resource<Texture>(&checkerTexture);

        DescriptorInfo storageImageInfo(DescriptorType::STORAGE_IMAGE, 3, ShaderType::FRAGMENT_SHADER);
        storageImageInfo.attach_resource<StorageImage>(&storageImage);

        DescriptorInfo storageImageQuadInfo(DescriptorType::STORAGE_IMAGE, 0, ShaderType::FRAGMENT_SHADER);
        storageImageQuadInfo.attach_resource<StorageImage>(&storageImage);

        set_per_frame.clear();
        post_process_set_per_frame.clear();
        set_per_frame.resize(3);
        post_process_set_per_frame.resize(3);
        for (size_t i = 0; i < 3; i++) {
            set_per_frame[i].Init({ checkerTexInfo, uboInfo, gridTexInfo, storageImageInfo });
            post_process_set_per_frame[i].Init({ storageImageQuadInfo });
        }
    }

    void BuildFixedPipeline() override {
        // Create the push constants
        modelPushConstant.stageFlags = ShaderType::VERTEX_SHADER | ShaderType::FRAGMENT_SHADER;
        modelPushConstant.offset = 0;
        modelPushConstant.size = sizeof(ModelPushConstant);

        geomFixedFunctions.SetFixedPipelineStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, baseSwapchain.get_extent(), false);
        geomFixedFunctions.SetPipelineLayout(set_per_frame[0].get_set_layout(), &modelPushConstant);

        postFixedFunctions.SetFixedPipelineStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, baseSwapchain.get_extent(), false);
        postFixedFunctions.SetPipelineLayout(post_process_set_per_frame[0].get_set_layout(), nullptr);
    }

    void BuildGraphicsPipeline() override {
        geomPassPipeline.Create(subdivisionShaders, geomFixedFunctions, baseRenderPass.get_handle());

        postProcessPipeline.Create(quadShaders, postFixedFunctions, baseRenderPass.get_handle());
    }

    void BuildFramebuffer() override {
        simpleFrameBuffer.Create(baseRenderPass.get_handle(), baseSwapchain.get_image_views(), depthImage.GetDepthImageView(), baseSwapchain.get_extent());
    }

    void CleanUpPipeline() override {
        ZoneScopedC(0xffffff);
        vkDeviceWaitIdle(VKDEVICE);
        for (size_t i = 0; i < 3; i++) {
            set_per_frame[i].Destroy();
            post_process_set_per_frame[i].Destroy();
        }
        set_per_frame.clear();
        post_process_set_per_frame.clear();
        simpleFrameBuffer.Destroy();
        storageImage.Destroy();
        gridTexture.Destroy();
        checkerTexture.Destroy();
        depthImage.Destroy();
        helloTriangleUBO.Destroy();
        helloTriangleVBO.Destroy();
        quadVB.Destroy();
        quadIB.Destroy();
        stanford_bunnyVB.Destroy();
        // stanford_bunnyIB.Destroy();
        geomPassPipeline.Destroy();
        postProcessPipeline.Destroy();
        geomFixedFunctions.DestroyPipelineLayout();
    }

private:

    void OnStart() override
    {

    }

    void OnRender(CmdBuffer dcb, CmdBuffer ccb) override
    {
        aspectRatio = (float)getWindow()->getWidth() / (float)getWindow()->getHeight();

        ZoneScopedC(0xffa500);
#ifdef OPTICK_ENABLE
        OPTICK_EVENT();
#endif
        baseRenderPass.set_clear_color(0.2f, 0.2f, 0.2f);
        auto framebuffers = simpleFrameBuffer.GetFramebuffers();
        //auto descriptorSets = helloTriangleUBO.GetSets();
#ifdef OPTICK_ENABLE
        OPTICK_GPU_CONTEXT(dcb.get_handle());
        OPTICK_GPU_EVENT("Recording cmd buffers");
#endif

        storageImage.clear(glm::vec4(0.0f));

        ccb.begin_recording();
        ccb.end_recording();

        dcb.begin_recording();
        baseRenderPass.begin_pass(dcb.get_handle(), framebuffers[get_image_idx()], baseSwapchain.get_extent());

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(getWindow()->getWidth());
        viewport.height = static_cast<float>(getWindow()->getHeight());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = { getWindow()->getWidth(), getWindow()->getHeight() };

        vkCmdSetViewport(dcb.get_handle(), 0, 1, &viewport);
        vkCmdSetScissor(dcb.get_handle(), 0, 1, &scissor);

        geomPassPipeline.Bind(dcb.get_handle());

        // Bind the appropriate descriptor sets
        auto vk_set = set_per_frame[get_frame_idx()].get_set();
        vkCmdBindDescriptorSets(dcb.get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, geomFixedFunctions.GetPipelineLayout(), 0, 1, &vk_set, 0, nullptr);

        // vkCmdDraw(dcb.get_handle(), rainbowTriangleVertices.size(), 1, 0, 0);
        //helloTriangleVBO.bind(dcb.get_handle());
        stanford_bunnyVB.bind(dcb.get_handle());
        // stanford_bunnyIB.bind(dcb.get_handle());


        // Update the size properly
        // Bind the push constants with the appropriate size
        // modelPCData.model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        modelPCData.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.005f));
        vkCmdPushConstants(dcb.get_handle(), geomFixedFunctions.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ModelPushConstant), &modelPCData);
        // Draw stuff
        for (auto& part : bunnyMesh.parts) {
            vkCmdDraw(dcb.get_handle(), part.VertexCount, 1, part.VertexOffset, 0);
        }
        // vkCmdDrawIndexed(dcb.get_handle(), stanford_bunnyIndices.size(), 1, 0, 0, 0);

        //--------------------------------
        // Post Process pass
        postProcessPipeline.Bind(dcb.get_handle());

        // Bind the appropriate descriptor sets
        auto vk_pp_set = post_process_set_per_frame[get_frame_idx()].get_set();
        vkCmdBindDescriptorSets(dcb.get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, postFixedFunctions.GetPipelineLayout(), 0, 1, &vk_pp_set, 0, nullptr);

        // vkCmdDraw(dcb.get_handle(), rainbowTriangleVertices.size(), 1, 0, 0);
        quadVB.bind(dcb.get_handle());
        quadIB.bind(dcb.get_handle());

        // Draw stuff
        vkCmdDrawIndexed(dcb.get_handle(), 6, 1, 0, 0, 0);

        ImGuiIO& io = ImGui::GetIO();

        io.DisplaySize = ImVec2((float)getWindow()->getWidth(), (float)getWindow()->getHeight());

        get_ui_overlay().update_imgui_buffers();
        get_ui_overlay().draw(dcb.get_handle());

        baseRenderPass.end_pass(dcb.get_handle());
        dcb.end_recording();
    }

    void OnUpdateBuffers(uint32_t frameIdx) override {
        vpUBOData.view = glm::mat4(1.0f);
        vpUBOData.proj = glm::mat4(1.0f);

        vpUBOData.view = getCamera().GetViewMatrix();
        vpUBOData.proj = glm::perspective(glm::radians(someNum), (float)baseSwapchain.get_extent().width / baseSwapchain.get_extent().height, 0.01f, 100.0f);
        vpUBOData.proj[1][1] *= -1;

        helloTriangleUBO.update_buffer(&vpUBOData, sizeof(ViewProjectionUBOData));
    }

    void OnImGui() override
    {
        ImGui::NewFrame();

        if (ImGui::Begin(get_app_name().c_str()))
        {
            ImGui::Text("FPS: %d | Avg : %d | Max : %d | Min : %d", get_fps(), avgFPS, maxFPS, minFPS);
            ImGui::Image((void*)gridTexture.get_descriptor_set(), ImVec2(50, 50), ImVec2(0, 0), ImVec2(1.0f, -1.0f)); ImGui::SameLine();
            ImGui::Image((void*)checkerTexture.get_descriptor_set(), ImVec2(50, 50), ImVec2(0, 0), ImVec2(1.0f, -1.0f));

            ImGui::Text("Descriptor Set Allocations : %d", DescriptorSet::get_current_set_allocations());
        }
        ImGui::End();

        ImGui::Render();
    }
};

VULF_MAIN(OverDrawVis)
