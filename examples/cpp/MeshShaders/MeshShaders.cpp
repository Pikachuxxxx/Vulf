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
   "VK_NV_mesh_shader"
#if (__APPLE__)
   "VK_KHR_portability_subset"
#endif
};

using namespace Vulf;

// Load some functions dynamically
//VKAPI_ATTR void VKAPI_CALL vkCmdDrawMeshTasksNV(
//    VkCommandBuffer                             commandBuffer,
//    uint32_t                                    taskCount,
//    uint32_t                                    firstTask);


void CmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask)
{
    auto func = (PFN_vkCmdDrawMeshTasksNV) vkGetDeviceProcAddr(Device::Get()->get_handle(), "vkCmdDrawMeshTasksNV");
    if (func != nullptr)
        func(commandBuffer, taskCount, firstTask);
    else
        VK_ERROR("cannot load vkCmdDrawMeshTasksNV function!");
}

class VulfMeshShaders : public Vulf::VulfBase
{
public:
    VulfMeshShaders() : VulfBase("Mesh Shaders") {}

    ~VulfMeshShaders()
    {
        VK_LOG("Exiting application...!");
        CleanUpPipeline();
    }

    // Types
private:
    struct ModelPushConstant
    {
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

    GraphicsPipeline        meshShaderPipeline;

    DepthImage              depthImage;

    Framebuffer             simpleFrameBuffer;

    using ShaderStages = std::vector<VkPipelineShaderStageCreateInfo>;
    // Shaders
    Shader                  meshShader;
    Shader                  meshPixelShader;
    ShaderStages             meshShadersStages;

private:
    void LoadShaders() override
    {
        VK_ERROR("Testing this");

        // Default shaders
        meshShader.Init((SHADER_BINARY_DIR) + std::string("/mesh_shader_mesh.spv"), ShaderType::MESH_SHADER);
        meshPixelShader.Init((SHADER_BINARY_DIR) + std::string("/mesh_shader_frag.spv"), ShaderType::FRAGMENT_SHADER);
        meshShadersStages.push_back(meshShader.get_shader_stage_info());
        meshShadersStages.push_back(meshPixelShader.get_shader_stage_info());
    }

    void BuildTextureResources() override
    {
        // default
        depthImage.CreateDepthImage(_def_Swapchain.GetSwapExtent().width, _def_Swapchain.GetSwapExtent().height, _def_CommandPool);
    }

    void BuildBufferResource() override
    {
        VK_LOG("");
    }

    void BuildFixedPipeline() override
    {
        // create one with an empty descriptor set
        fixedFunctions.SetFixedPipelineStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, _def_Swapchain.GetSwapExtent(), false);
        fixedFunctions.SetPipelineLayout(VK_NULL_HANDLE, nullptr);
    }

    void BuildGraphicsPipeline() override
    {
        meshShaderPipeline.Create(meshShadersStages, fixedFunctions, _def_RenderPass.GetRenderPass());
    }

    // default
    void BuildFramebuffer() override
    {
        simpleFrameBuffer.Create(_def_RenderPass.GetRenderPass(), _def_Swapchain.GetSwapImageViews(), depthImage.GetDepthImageView(), _def_Swapchain.GetSwapExtent());
    }

    void CleanUpPipeline() override
    {
        ZoneScopedC(0xffffff);
        vkDeviceWaitIdle(VKDEVICE);
        simpleFrameBuffer.Destroy();
        depthImage.Destroy();
        _def_RenderPass.Destroy();
        fixedFunctions.DestroyPipelineLayout();
        _def_Swapchain.Destroy();
    }

private:

    void OnRender(VkCommandBuffer commandBuffer, uint32_t imageIndex) override
    {
        ZoneScopedC(0xffa500);
        OPTICK_EVENT();

        _def_RenderPass.set_clear_color(0.0f, 0.0f, 0.0f);
        auto framebuffers = simpleFrameBuffer.GetFramebuffers();

        int i = imageIndex;

#ifdef _WIN32
        OPTICK_GPU_CONTEXT(commandBuffer);
        OPTICK_GPU_EVENT("Recording cmd buffers");
#endif
        _def_SubmissionCommandBuffers.RecordBuffer(commandBuffer);
        _def_RenderPass.begin_pass(commandBuffer, framebuffers[i], _def_Swapchain.GetSwapExtent());

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

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        meshShaderPipeline.Bind(commandBuffer);

        uint32_t num_workgroups = 1;
        CmdDrawMeshTasksNV(commandBuffer, num_workgroups, 0);

        ImGuiIO& io = ImGui::GetIO();

        io.DisplaySize = ImVec2((float) getWindow()->getWidth(), (float) getWindow()->getHeight());

        get_ui_overlay().update_imgui_buffers();
        get_ui_overlay().draw(commandBuffer);

        _def_RenderPass.end_pass(commandBuffer);
        _def_SubmissionCommandBuffers.EndRecordingBuffer(commandBuffer);
    }

    void OnImGui() override
    {
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        if (ImGui::Begin("Vulkan Example")) {
            ImGui::Text("Hello ImGui");
            ImGui::TextUnformatted(get_app_name().c_str());
            ImGui::DragFloat("Position", &someNum, 0.1f, 0.0f, 100.0f);
        }
        ImGui::End();

        ImGui::Render();
    }
};

VULF_MAIN(MeshShaders)
