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
     "VK_KHR_dynamic_rendering",
     "VK_KHR_depth_stencil_resolve",
     "VK_KHR_create_renderpass2",
#if (__APPLE__)
   "VK_KHR_portability_subset"
#endif
};

using namespace Vulf;

static void CmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo)
{
    auto func = (PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(VKDEVICE, "vkCmdBeginRenderingKHR");
    if (func != nullptr)
        func(commandBuffer, pRenderingInfo);
}

static void CmdEndRenderingKHR(VkCommandBuffer commandBuffer)
{
    auto func = (PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(VKDEVICE, "vkCmdEndRenderingKHR");
    if (func != nullptr)
        func(commandBuffer);
}

class VulfHelloTriangle : public Vulf::VulfBase
{
public:
    VulfHelloTriangle() : VulfBase("Hello Triangle") {}

    ~VulfHelloTriangle() {
        VK_LOG("Quitting...");
        defaultVertShader.Destroy();
        defaultFragShader.Destroy();
    }

    // Types
private:
    struct ModelPushConstant
    {
        alignas(16) glm::mat4 model = glm::mat4(1.0f);
    }modelPCData;

    struct ViewProjectionUBOData
    {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::mat4 _padding1 = glm::mat4(0.0f);
        alignas(16) glm::mat4 _padding2 = glm::mat4(0.0f);
    }vpUBOData;

    float someNum = 45.0f;
    bool    useOrtho = false;
    float aspectRatio = 1280 / 720;
private:
    // default stuff required for initialization, these resources are all explicitly allocated and to not follow RAII, hence the defauly ones are provided by Vulf
    FixedPipelineFuncs      fixedFunctions;

    GraphicsPipeline        simpleGraphicsPipeline;

    VkPushConstantRange     modelPushConstant;

    DepthImage              depthImage;

    Framebuffer             simpleFrameBuffer;

    using ShaderStages = std::vector<VkPipelineShaderStageCreateInfo>;
    // Shaders
    Shader                  defaultVertShader;
    Shader                  defaultFragShader;
    ShaderStages             defaultShaders;

    // Buffers
    VertexBuffer            helloTriangleVBO;
    UniformBuffer           helloTriangleUBO;

    // Textures
    Texture                 gridTexture;
    Texture                 checkerTexture;
    std::vector<DescriptorSet>           set_per_frame;
private:
    void LoadShaders() override
    {

        // Default shaders
        defaultVertShader.Init((SHADER_BINARY_DIR)+std::string("/default.vert.spv"), ShaderType::VERTEX_SHADER);
        defaultFragShader.Init((SHADER_BINARY_DIR)+std::string("/default.frag.spv"), ShaderType::FRAGMENT_SHADER);
        defaultShaders.push_back(defaultVertShader.get_shader_stage_info());
        defaultShaders.push_back(defaultFragShader.get_shader_stage_info());
    }

    void BuildTextureResources() override
    {
        // default
        depthImage.CreateDepthImage(baseSwapchain.get_extent().width, baseSwapchain.get_extent().height, graphicsCommandPool);
    }

    void BuildBufferResource() override
    {
        // Triangle vertices and indices
        helloTriangleVBO.Init(rainbowTriangleVertices);
        std::vector<Vertex> vertices;
        vertices.resize(3);
        vertices[0].position = glm::vec3(-0.5f, 0.5f, 0.0f);
        vertices[0].color = glm::vec3(1, 0, 0);
        vertices[1].position = glm::vec3(0.0f, -0.5f, 0.0f);
        vertices[1].color = glm::vec3(0, 1, 0);
        vertices[2].position = glm::vec3(0.5f, 0.5f, 0.0f);
        vertices[2].color = glm::vec3(1, 0, 1);

        helloTriangleVBO.Init(vertices);

        // Shader buffer resources
        {
            helloTriangleUBO.Init(sizeof(ViewProjectionUBOData));
        }

        // Descriptors bindings info and resource mapping
        {
            DescriptorInfo vpuboInfo(DescriptorType::UNIFORM_BUFFER, 0, ShaderType::VERTEX_SHADER);
            vpuboInfo.attach_resource<UniformBuffer>(&helloTriangleUBO);

            set_per_frame.clear();
            set_per_frame.resize(3);
            for (size_t i = 0; i < 3; i++)
                set_per_frame[i].Init({ vpuboInfo });
        }
    }

    void BuildFixedPipeline() override
    {
        // Create the push constants
        modelPushConstant.stageFlags = ShaderType::VERTEX_SHADER | ShaderType::FRAGMENT_SHADER;
        modelPushConstant.offset = 0;
        modelPushConstant.size = sizeof(ModelPushConstant);

        fixedFunctions.SetFixedPipelineStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, baseSwapchain.get_extent(), false);
        fixedFunctions.SetPipelineLayout(set_per_frame[0].get_set_layout(), &modelPushConstant);
        fixedFunctions.SetRasterizerSCI(VK_CULL_MODE_NONE);
        fixedFunctions.SetDepthStencilSCI(VK_COMPARE_OP_LESS_OR_EQUAL);
    }

    void BuildGraphicsPipeline() override
    {
        simpleGraphicsPipeline.Create(defaultShaders, fixedFunctions, baseRenderPass.get_handle());
    }

    // default
    void BuildFramebuffer() override
    {
        simpleFrameBuffer.Create(baseRenderPass.get_handle(), baseSwapchain.get_image_views(), depthImage.GetDepthImageView(), baseSwapchain.get_extent());
    }

    void CleanUpPipeline() override
    {
        ZoneScopedC(0xffffff);
        vkDeviceWaitIdle(VKDEVICE);
        for (size_t i = 0; i < 3; i++) {
            set_per_frame[i].Destroy();
        }
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

    void OnRender(CmdBuffer dcb, CmdBuffer ccb) override
    {
        aspectRatio = (float)getWindow()->getWidth() / (float)getWindow()->getHeight();

        ZoneScopedC(0xffa500);

        auto framebuffers = simpleFrameBuffer.GetFramebuffers();

        ccb.begin_recording();
        ccb.end_recording();

        //----------------------------------------------------------------------

        dcb.begin_recording();

        // Begin Rendering Info
        VkClearValue screenClear;
        screenClear.color = { 0.1f, 0.1f, 0.1f, 1.0f };
        screenClear.depthStencil = { 1.0f };

        const VkRenderingAttachmentInfoKHR color_attachment_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = baseSwapchain.get_image_view_at(get_image_idx()),
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = screenClear
        };

        VkRect2D renderArea{};
        renderArea.extent.width = getWindow()->getWidth();
        renderArea.extent.height = getWindow()->getHeight();

        const VkRenderingInfoKHR render_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .renderArea = renderArea,
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_info,
        };

        CmdBeginRenderingKHR(dcb.get_handle(), &render_info);

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

        simpleGraphicsPipeline.Bind(dcb.get_handle());

        // Bind the appropriate descriptor sets
        auto vk_pp_set = set_per_frame[get_frame_idx()].get_set();
        vkCmdBindDescriptorSets(dcb.get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, fixedFunctions.GetPipelineLayout(), 0, 1, &vk_pp_set, 0, nullptr);

        helloTriangleVBO.bind(dcb.get_handle());

        vkCmdPushConstants(dcb.get_handle(), fixedFunctions.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ModelPushConstant), &modelPCData);

        vkCmdDraw(dcb.get_handle(), helloTriangleVBO.get_vtx_count(), 1, 0, 0);

        ImGuiIO& io = ImGui::GetIO();

        io.DisplaySize = ImVec2((float)getWindow()->getWidth(), (float)getWindow()->getHeight());

        //get_ui_overlay().update_imgui_buffers();
        //get_ui_overlay().draw(dcb.get_handle());
        CmdEndRenderingKHR(dcb.get_handle());
        dcb.end_recording();
    }

    void OnUpdateBuffers(uint32_t frameIdx) override
    {
        vpUBOData.view = glm::mat4(1.0f);
        vpUBOData.proj = glm::mat4(1.0f);

        helloTriangleUBO.update_buffer(&vpUBOData, sizeof(ViewProjectionUBOData));
    }

    void OnImGui() override
    {
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        if (ImGui::Begin("Vulkan Example")) {
            ImGui::TextUnformatted(get_app_name().c_str());
            ImGui::Text("FPS: %d | dt: %f", get_fps(), get_dt());
        }
        ImGui::End();

        ImGui::Render();
    }
};

VULF_MAIN(HelloTriangle)
