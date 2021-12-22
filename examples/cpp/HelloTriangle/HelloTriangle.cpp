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

private:
    // Basic Stuff
    FixedPipelineFuncs      fixedFunctions;

    RenderPass              simpleRenderPass;

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

    // default
    void BuildRenderPass() override {
        simpleRenderPass.Init(_def_Swapchain.GetSwapFormat());
    }

    void BuildGraphicsPipeline() override {
        simpleGraphicsPipeline.Create(defaultShaders, fixedFunctions, simpleRenderPass.GetRenderPass());
    }

    // default
    void BuildFramebuffer() override {
        simpleFrameBuffer.Create(simpleRenderPass.GetRenderPass(), _def_Swapchain.GetSwapImageViews(), depthImage.GetDepthImageView(), _def_Swapchain.GetSwapExtent());
    }

    // default
    void BuildCommandBuffers() override {
        simpleCommandBuffer.AllocateBuffers(_def_CommandPool.GetPool());

    }

    void UpdateBuffers(uint32_t imageIndex) override {
        vpUBOData.view = glm::mat4(1.0f);
        vpUBOData.proj = glm::mat4(1.0f);

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
        simpleRenderPass.Destroy();
        fixedFunctions.DestroyPipelineLayout();
        _def_Swapchain.Destroy();
    }


private:

    void OnStart() override
    {

        simpleRenderPass.SetClearColor(0.0f, 0.0f, 0.0f);
        auto& cmdBuffers = simpleCommandBuffer.GetBuffers();
        auto framebuffers = simpleFrameBuffer.GetFramebuffers();
        auto descriptorSets = helloTriangleUBO.GetSets();

        for (int i = 0; i < cmdBuffers.size(); i++) {

#ifdef _WIN32
            OPTICK_GPU_CONTEXT(cmdBuffers[i]);
            OPTICK_GPU_EVENT("Recording cmd buffers");
#endif
            simpleCommandBuffer.RecordBuffer(cmdBuffers[i]);
            simpleRenderPass.BeginRenderPass(cmdBuffers[i], framebuffers[i], _def_Swapchain.GetSwapExtent());

            simpleGraphicsPipeline.Bind(cmdBuffers[i]);

            // Bind the appropriate descriptor sets
            vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, fixedFunctions.GetPipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);

            // Bind the push constants
            modelPCData.model = glm::rotate(glm::mat4(1.0f), (float) glm::radians(90.0f * 2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            vkCmdPushConstants(cmdBuffers[i], fixedFunctions.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ModelPushConstant), &modelPCData);

            helloTriangleVBO.Bind(cmdBuffers[i]);

            vkCmdDraw(cmdBuffers[i], rainbowTriangleVertices.size(), 1, 0, 0);

            simpleRenderPass.EndRenderPass(cmdBuffers[i]);
            simpleCommandBuffer.EndRecordingBuffer(cmdBuffers[i]);
        }

        submissionCommandBuffers.clear();
        submissionCommandBuffers.push_back(simpleCommandBuffer);
    }

    void OnRender() override
    {
        ZoneScopedC(0xffa500);
        OPTICK_EVENT();
    }
};

VULF_MAIN(HelloTriangle)
