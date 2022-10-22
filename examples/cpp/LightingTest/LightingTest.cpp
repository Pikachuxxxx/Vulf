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

// TODO: 
// [ ] Compute Command Buffer + Pool
// [ ] Compute Queue submission (no need to sync their submit operations we do them asynchronously and use mem barriers for the resources)

class VulfLightingTest : public Vulf::VulfBase
{
public:
    VulfLightingTest() : VulfBase("Lighting Test")
    {
        LoadObjModel((SRC_DIR)+std::string("/data/models/stanford-bunny.obj"), stanford_bunnyVertices, stanford_bunnyIndices);
        knotMesh = LoadObjModel((SRC_DIR)+std::string("/data/models/knot.obj"));
    }

    ~VulfLightingTest() {
        VK_LOG("Quitting...");
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

    struct DirectionalLightingData
    {
        glm::vec3 direction = glm::vec3(1.0f);
        glm::vec3 ambient = glm::vec3(1.0f);
        glm::vec3 diffuse = glm::vec3(1.0f);
        glm::vec3 specular = glm::vec3(1.0f);
        glm::vec4 _padding;
    }dirLightData;

    float aspectRatio = 1280 / 720;

private:
    // default stuff required for initialization, these resources are all explicitly allocated and to not follow RAII, hence the defauly ones are provided by Vulf
    FixedPipelineFuncs      lightingFixedFunctions;

    GraphicsPipeline        lightingPipeline;

    VkPushConstantRange     modelPushConstant;

    DepthImage              depthImage;

    Framebuffer             simpleFrameBuffer;

    using ShaderStages = std::vector<VkPipelineShaderStageCreateInfo>;
    // Shaders

    Shader                  defaultVertShader;
    Shader                  phongLightingShader;

    ShaderStages            lightingShaderStages;

    // Buffers
    UniformBuffer           vpUBO;
    UniformBuffer           lightingUBO;

    VertexBuffer            planeVB;
    IndexBuffer             planeIB;

    Mesh                    knotMesh;
    VertexBuffer            knotVB;

    std::vector<Vertex>     stanford_bunnyVertices;
    std::vector<uint16_t>   stanford_bunnyIndices;

    VertexBuffer            stanford_bunnyVB;
    IndexBuffer             stanford_bunnyIB;

    std::vector<DescriptorSet>           set_per_frame;
private:
    void LoadShaders() override {

        // Default shaders

        defaultVertShader.Init((SHADER_BINARY_DIR)+std::string("/defaultVert.spv"), ShaderType::VERTEX_SHADER);
        phongLightingShader.Init((SHADER_BINARY_DIR)+std::string("/phong.spv"), ShaderType::FRAGMENT_SHADER);

        lightingShaderStages.push_back(defaultVertShader.get_shader_stage_info());
        lightingShaderStages.push_back(phongLightingShader.get_shader_stage_info());
    }

    void BuildTextureResources() override {
        // default
        depthImage.CreateDepthImage(baseSwapchain.get_extent().width, baseSwapchain.get_extent().height, graphicsCommandPool);
    }

    void BuildBufferResource() override {

        vpUBO.Init(sizeof(ViewProjectionUBOData));
        lightingUBO.Init(sizeof(DirectionalLightingData));

        // Quad VB & IB
        planeVB.Init(planeVertices);
        planeIB.Init(planeIndices);

        stanford_bunnyVB.Init(stanford_bunnyVertices);
        stanford_bunnyIB.Init(stanford_bunnyIndices);

        knotVB.Init(knotMesh.vertices);

        DescriptorInfo vpuboInfo(DescriptorType::UNIFORM_BUFFER, 0, ShaderType::VERTEX_SHADER);
        vpuboInfo.attach_resource<UniformBuffer>(&vpUBO);

        DescriptorInfo lightuboInfo(DescriptorType::UNIFORM_BUFFER, 1, ShaderType::FRAGMENT_SHADER);
        lightuboInfo.attach_resource<UniformBuffer>(&lightingUBO);


        set_per_frame.clear();
        set_per_frame.resize(3);
        for (size_t i = 0; i < 3; i++) {
            set_per_frame[i].Init({ vpuboInfo, lightuboInfo });
        }
    }

    void BuildFixedPipeline() override {
        // Create the push constants
        modelPushConstant.stageFlags = ShaderType::VERTEX_SHADER | ShaderType::FRAGMENT_SHADER;
        modelPushConstant.offset = 0;
        modelPushConstant.size = sizeof(ModelPushConstant);

        lightingFixedFunctions.SetFixedPipelineStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, baseSwapchain.get_extent(), false);
        lightingFixedFunctions.SetPipelineLayout(set_per_frame[0].get_set_layout(), &modelPushConstant);

        //lightingFixedFunctions.SetRasterizerSCI(true);
    }

    void BuildGraphicsPipeline() override {

        lightingPipeline.Create(lightingShaderStages, lightingFixedFunctions, baseRenderPass.get_handle());
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
        planeVB.Destroy();
        planeIB.Destroy();
        knotVB.Destroy();
        stanford_bunnyVB.Destroy();
        stanford_bunnyIB.Destroy();
        lightingPipeline.Destroy();
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

        lightingPipeline.Bind(dcb.get_handle());

        // Bind the appropriate descriptor sets
        auto vk_pp_set = set_per_frame[get_frame_idx()].get_set();
        vkCmdBindDescriptorSets(dcb.get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, lightingFixedFunctions.GetPipelineLayout(), 0, 1, &vk_pp_set, 0, nullptr);

        planeVB.bind(dcb.get_handle());
        planeIB.bind(dcb.get_handle());

        modelPCData.model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
        vkCmdPushConstants(dcb.get_handle(), lightingFixedFunctions.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ModelPushConstant), &modelPCData);

        INSERT_MARKER(dcb, "Draw Plane", glm::vec4(0.2f));
        // Draw stuff
        vkCmdDrawIndexed(dcb.get_handle(), 6, 1, 0, 0, 0);

        INSERT_MARKER(dcb, "Draw Knot", glm::vec4(0.2, 0.6f, 0.4f, 1.0f));

        knotVB.bind(dcb.get_handle());
        modelPCData.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));
        vkCmdPushConstants(dcb.get_handle(), lightingFixedFunctions.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ModelPushConstant), &modelPCData);
        for (auto& part : knotMesh.parts)
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
        //vpUBOData.proj[1][1] *= -1;

        vpUBO.update_buffer(&vpUBOData, sizeof(ViewProjectionUBOData));

        // Update Lighting Data
        lightingUBO.update_buffer(&dirLightData, sizeof(DirectionalLightingData));
    }

    void OnImGui() override
    {
        ImGui::NewFrame();

        if (ImGui::Begin(get_app_name().c_str()))
        {
            ImGui::Text("FPS: %d | Avg : %d | Max : %d | Min : %d", get_fps(), avgFPS, maxFPS, minFPS);
            ImGui::Text("Descriptor Set Allocations : %d", DescriptorSet::get_current_set_allocations());
        }
        ImGui::End();

        ImGui::Render();
    }
};

VULF_MAIN(LightingTest)
