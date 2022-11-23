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

class VulfLightingTest : public Vulf::VulfBase
{
public:
    VulfLightingTest() : VulfBase("Lighting Test")
    {
        teapotMesh = LoadObjModel((SRC_DIR)+std::string("/data/models/teapot.obj"));

        GenerateSphereSmooth(5, 10, 10);
    }

    ~VulfLightingTest() {
        VK_LOG("Quitting...");
    }

    // Types
private:
    struct ModelPushConstant {
        alignas(16) glm::mat4 model;
    }modelPCData;

    struct LightUBOData {
        glm::vec4 viewPos;
    }lightUBOData;

    struct ViewProjectionUBOData
    {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::mat4 _padding1 = glm::mat4(0.0f);
        alignas(16) glm::mat4 _padding2 = glm::mat4(0.0f);
    }vpUBOData;

    struct DirectionalLightingData
    {
        glm::vec4 direction = glm::vec4(1.0f);
        glm::vec4 ambient   = glm::vec4(glm::vec3(0.015), 1.0f);
        glm::vec4 diffuse   = glm::vec4(0.94, 0.1 , 0.14, 1.0f);
        glm::vec4 specular  = glm::vec4(1.0f);
        glm::vec4 viewPos;
    }dirLightData;

    float aspectRatio = 1280 / 720;

private:
    // default stuff required for initialization, these resources are all explicitly allocated and to not follow RAII, hence the defauly ones are provided by Vulf
    FixedPipelineFuncs      skyboxFixedFunctions;
    GraphicsPipeline        skyboxPipeline;

    FixedPipelineFuncs      geomFixedFunctions;
    GraphicsPipeline        geomPipeline;

    VkPushConstantRange     modelPushConstant;

    DepthImage              depthImage;

    Framebuffer             simpleFrameBuffer;

    using ShaderStages = std::vector<VkPipelineShaderStageCreateInfo>;
    // Shaders

    Shader                  skyboxVertShader;
    Shader                  skyboxFragShader;
    Shader                  defaultVertShader;
    Shader                  reflectionShader;

    ShaderStages            skyboxShaderStages;
    ShaderStages            reflectionShaderStages;

    // Buffers
    UniformBuffer           vpUBO;
    UniformBuffer           lightingUBO;

    VertexBuffer            cubeVB;

    Mesh                    teapotMesh;
    VertexBuffer            teapotVB;

    Texture                 skyboxTexture;

    std::vector<DescriptorSet>           set_per_frame;
private:
    void LoadShaders() override {

        // Skybox shaders
        skyboxVertShader.Init((SHADER_BINARY_DIR)+std::string("/skybox.vert.spv"), ShaderType::VERTEX_SHADER);
        skyboxFragShader.Init((SHADER_BINARY_DIR)+std::string("/skybox.frag.spv"), ShaderType::FRAGMENT_SHADER);
        skyboxShaderStages.push_back(skyboxVertShader.get_shader_stage_info());
        skyboxShaderStages.push_back(skyboxFragShader.get_shader_stage_info());

        defaultVertShader.Init((SHADER_BINARY_DIR)+std::string("/default.vert.spv"), ShaderType::VERTEX_SHADER);
        reflectionShader.Init((SHADER_BINARY_DIR)+std::string("/reflection.frag.spv"), ShaderType::FRAGMENT_SHADER);
        reflectionShaderStages.push_back(defaultVertShader.get_shader_stage_info());
        reflectionShaderStages.push_back(reflectionShader.get_shader_stage_info());
    }

    void BuildTextureResources() override {

        skyboxTexture.Init((SRC_DIR)+std::string("/data/textures/HDR/table_mountain.hdr"));

        // default
        depthImage.CreateDepthImage(baseSwapchain.get_extent().width, baseSwapchain.get_extent().height, graphicsCommandPool);
    }

    void BuildBufferResource() override {

        // Shader buffer resources
        {
            vpUBO.Init(sizeof(ViewProjectionUBOData));
            lightingUBO.Init(sizeof(LightUBOData));
        }

        // Meshes
        {
            // Quad VB & IB
            cubeVB.Init(cubeVertices);

            teapotVB.Init(teapotMesh.vertices);
        }

        // Descriptors bindings info and resouce mapping
        {
            DescriptorInfo vpuboInfo(DescriptorType::UNIFORM_BUFFER, 0, ShaderType::VERTEX_SHADER);
            vpuboInfo.attach_resource<UniformBuffer>(&vpUBO);

            DescriptorInfo skyboxTexInfo(DescriptorType::COMBINED_IMAGE_SAMPLER, 1, ShaderType::FRAGMENT_SHADER);
            skyboxTexInfo.attach_resource<Texture>(&skyboxTexture);

            DescriptorInfo lightuboInfo(DescriptorType::UNIFORM_BUFFER, 2, ShaderType::FRAGMENT_SHADER);
            lightuboInfo.attach_resource<UniformBuffer>(&lightingUBO);

            set_per_frame.clear();
            set_per_frame.resize(3);
            for (size_t i = 0; i < 3; i++)
                set_per_frame[i].Init({ vpuboInfo, skyboxTexInfo, lightuboInfo });
        }
    }

    void BuildFixedPipeline() override {
        // Create the push constants
        modelPushConstant.stageFlags = ShaderType::VERTEX_SHADER | ShaderType::FRAGMENT_SHADER;
        modelPushConstant.offset = 0;
        modelPushConstant.size = sizeof(ModelPushConstant);

        skyboxFixedFunctions.SetFixedPipelineStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, baseSwapchain.get_extent(), false);
        skyboxFixedFunctions.SetPipelineLayout(set_per_frame[0].get_set_layout(), &modelPushConstant);
        skyboxFixedFunctions.SetRasterizerSCI(VK_CULL_MODE_NONE);
        skyboxFixedFunctions.SetDepthStencilSCI(VK_COMPARE_OP_LESS_OR_EQUAL);

        geomFixedFunctions.SetFixedPipelineStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, baseSwapchain.get_extent(), false);
        geomFixedFunctions.SetPipelineLayout(set_per_frame[0].get_set_layout(), &modelPushConstant);

    }

    void BuildGraphicsPipeline() override {

        skyboxPipeline.Create(skyboxShaderStages, skyboxFixedFunctions, baseRenderPass.get_handle());

        geomPipeline.Create(reflectionShaderStages, geomFixedFunctions, baseRenderPass.get_handle());
    }

    void BuildFramebuffer() override {
        simpleFrameBuffer.Create(baseRenderPass.get_handle(), baseSwapchain.get_image_views(), depthImage.GetDepthImageView(), baseSwapchain.get_extent());
    }

    void CleanUpPipeline() override {
        ZoneScopedC(0xffffff);
        vkDeviceWaitIdle(VKDEVICE);
        for (size_t i = 0; i < 3; i++) {
            set_per_frame[i].Destroy();
        }
        set_per_frame.clear();
        simpleFrameBuffer.Destroy();
        depthImage.Destroy();
        vpUBO.Destroy();
        lightingUBO.Destroy();
        cubeVB.Destroy();
        teapotVB.Destroy();
        skyboxPipeline.Destroy();
    }

private:

    void OnStart() override
    {
        // generate cubemap from equirectangularMap

    }

    void OnRender(CmdBuffer dcb, CmdBuffer ccb) override
    {
        aspectRatio = (float)getWindow()->getWidth() / (float)getWindow()->getHeight();

        ZoneScopedC(0xffa500);
#ifdef OPTICK_ENABLE
        OPTICK_EVENT();
#endif
        baseRenderPass.set_clear_color(0.2f, 0.2f, 0.2f); // 19, 50, 78
        auto framebuffers = simpleFrameBuffer.GetFramebuffers();

#ifdef OPTICK_ENABLE
        OPTICK_GPU_CONTEXT(dcb.get_handle());
        OPTICK_GPU_EVENT("Recording graphics cmd buffers");
#endif

        ccb.begin_recording();
        ccb.end_recording();

        //----------------------------------------------------------------------

        dcb.begin_recording();

        BEGIN_MARKER(dcb, "Lighting Pass", glm::vec4(0.4f, 0.8f, 0.6f, 1.0f));

        baseRenderPass.begin_pass(dcb.get_handle(), framebuffers[get_image_idx()], baseSwapchain.get_extent());

#if 1
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

        skyboxPipeline.Bind(dcb.get_handle());

        // Bind the appropriate descriptor sets
        auto vk_pp_set = set_per_frame[get_frame_idx()].get_set();
        vkCmdBindDescriptorSets(dcb.get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxFixedFunctions.GetPipelineLayout(), 0, 1, &vk_pp_set, 0, nullptr);

        cubeVB.bind(dcb.get_handle());

        modelPCData.model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
        vkCmdPushConstants(dcb.get_handle(), skyboxFixedFunctions.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ModelPushConstant), &modelPCData);

        INSERT_MARKER(dcb, "Draw Cube", glm::vec4(0.2f));
        // Draw stuff
        vkCmdDraw(dcb.get_handle(), 36, 1, 0, 0);

        INSERT_MARKER(dcb, "Draw Teapot", glm::vec4(0.2, 0.6f, 0.4f, 1.0f));

        geomPipeline.Bind(dcb.get_handle());

        vkCmdBindDescriptorSets(dcb.get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, geomFixedFunctions.GetPipelineLayout(), 0, 1, &vk_pp_set, 0, nullptr);

        teapotVB.bind(dcb.get_handle());

        modelPCData.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));
        vkCmdPushConstants(dcb.get_handle(), geomFixedFunctions.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ModelPushConstant), &modelPCData);
        for (auto& part : teapotMesh.parts)
            vkCmdDraw(dcb.get_handle(), part.VertexCount, 1, part.VertexOffset, 0);

        BEGIN_MARKER(dcb, "ImGui Pass", glm::vec4(0.94, 0.16, 0.08, 1.0f));

        ImGuiIO& io = ImGui::GetIO();

        io.DisplaySize = ImVec2((float)getWindow()->getWidth(), (float)getWindow()->getHeight());

        get_ui_overlay().update_imgui_buffers();
        get_ui_overlay().draw(dcb.get_handle());
        END_MARKER(dcb);
#endif
        baseRenderPass.end_pass(dcb.get_handle());
        END_MARKER(dcb);
        dcb.end_recording();
    }

    void OnUpdateBuffers(uint32_t frameIdx) override {
        // Update VP
        vpUBOData.view = glm::mat4(1.0f);
        vpUBOData.proj = glm::mat4(1.0f);

        vpUBOData.view = getCamera().GetViewMatrix();
        vpUBOData.proj = glm::perspective(glm::radians(45.0f), (float)baseSwapchain.get_extent().width / baseSwapchain.get_extent().height, 0.01f, 100.0f);
        vpUBOData.proj[1][1] *= -1;

        vpUBO.update_buffer(&vpUBOData, sizeof(ViewProjectionUBOData));
        //
        // // Update Lighting Data
        lightUBOData.viewPos = glm::vec4(getCamera().Position, 1.0f);
        lightingUBO.update_buffer(&lightUBOData, sizeof(LightUBOData));
    }

    void OnImGui() override
    {
        ImGui::NewFrame();

        if (ImGui::Begin(get_app_name().c_str()))
        {
            ImGui::Text("FPS: %d | Avg : %d | Max : %d | Min : %d", get_fps(), avgFPS, maxFPS, minFPS);
            ImGui::Text("Descriptor Set Allocations : %d", DescriptorSet::get_current_set_allocations());

            ImGui::Separator();

            ImGui::DragFloat3("Light Direction", glm::value_ptr(dirLightData.direction));
            ImGui::Text("Camera Pos : (%f, %f, %f) | YAW : %f | Pitch : %f", getCamera().Position.x, getCamera().Position.y, getCamera().Position.z, getCamera().Yaw ,getCamera().Pitch);
        }
        ImGui::End();

        ImGui::Render();
    }
};

VULF_MAIN(LightingTest)
