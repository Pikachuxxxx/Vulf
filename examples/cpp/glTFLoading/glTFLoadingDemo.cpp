#if 0
#include <iostream>

#include <VulfBase.h>
#include "utils/VulkanCheckResult.h"


// Load the Instance extensions/layers and device Extensions necessary for the peoject
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

#include <VulfBase.h>

class VulfglTFLoadingDemo : public Vulf::VulfBase
{
public:
    VulfglTFLoadingDemo() : VulfBase("glTF Loading Demo") {}

    ~VulfglTFLoadingDemo() {
        CleanUpPipeline();
        _def_CommandPool.Destroy();
        cubeVertShader.DestroyModule();
        cubeFragShader.DestroyModule();
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

private:
    // Basic Stuff
    FixedPipelineFuncs      fixedFunctions;

    RenderPass              simpleRenderPass;

    GraphicsPipeline        simpleGraphicsPipeline;

    VkPushConstantRange     modelPushConstant;

    DepthImage              depthImage;

    Framebuffer             simpleFrameBuffer;

    CmdBuffer               simpleCommandBuffer;

    using ShaderStages = std::vector<VkPipelineShaderStageCreateInfo>;
    // Shaders
    Shader                  cubeVertShader;
    Shader                  cubeFragShader;
    ShaderStages             cubeShaders;

    // Buffers
    UniformBuffer           helloTriangleUBO;

    // Textures
    Texture                 gridTexture;
    Texture                 checkerTexture;


private:
    void LoadShaders() override {

        // Default shaders
        cubeVertShader.CreateShader((SHADER_BINARY_DIR) + std::string("/cubeVert.spv"), ShaderType::VERTEX_SHADER);
        cubeFragShader.CreateShader((SHADER_BINARY_DIR) + std::string("/cubeFrag.spv"), ShaderType::FRAGMENT_SHADER);
        cubeShaders.push_back(cubeVertShader.GetShaderStageInfo());
        cubeShaders.push_back(cubeFragShader.GetShaderStageInfo());
    }

    void LoadAssets() override {
        // const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
        // testModel.loadFromFile((ASSETS_DIR) + std::string("/models/sponza/sponza.gltf"), Device::GetDeviceManager()->GetGraphicsQueue(), glTFLoadingFlags);
    }


    void BuildTextureResources() override {
        // default
        depthImage.CreateDepthImage(_def_Swapchain.GetSwapExtent().width, _def_Swapchain.GetSwapExtent().height, _def_CommandPool);

        // Grid Texture
        gridTexture.CreateTexture((SRC_DIR) +std::string("/data/textures/TestGrid_256.png"), _def_CommandPool);

        // Checker Texture;
        checkerTexture.CreateTexture((SRC_DIR) +std::string("/data/textures/TestCheckerMap.png"), _def_CommandPool);
    }

    void BuildBufferResource() override {
        // Triangle vertices and indices

        // View Projection Uniform Buffer
        helloTriangleUBO.AddDescriptor(UniformBuffer::DescriptorInfo(0, ShaderType::VERTEX_SHADER, sizeof(ViewProjectionUBOData), 0));
        helloTriangleUBO.AddDescriptor(UniformBuffer::DescriptorInfo(1, ShaderType::FRAGMENT_SHADER, gridTexture));
        helloTriangleUBO.CreateUniformBuffer(3, sizeof(ViewProjectionUBOData));
    }

    void BuildFixedPipeline() override {
        // Create the push constants
        modelPushConstant.stageFlags = ShaderType::VERTEX_SHADER | ShaderType::FRAGMENT_SHADER;
        modelPushConstant.offset = 0;
        modelPushConstant.size = sizeof(ModelPushConstant);

        fixedFunctions.SetFixedPipelineStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, _def_Swapchain.GetSwapExtent(), false);
        fixedFunctions.SetPipelineLayout(helloTriangleUBO.GetDescriptorSetLayout(), modelPushConstant);
    }

    // default
    void BuildRenderPass() override {
        simpleRenderPass.Init(_def_Swapchain.GetSwapFormat());
    }

    void BuildGraphicsPipeline() override {
        simpleGraphicsPipeline.Create(cubeShaders, fixedFunctions, simpleRenderPass.GetRenderPass());
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
        vpUBOData.view = getCamera().GetViewMatrix(); // glm::mat4(1.0f); //
        vpUBOData.proj = glm::perspective(45.0f, (float) _def_Swapchain.GetSwapExtent().width / (float) _def_Swapchain.GetSwapExtent().height, 0.01f, 100.0f);
        vpUBOData.proj[1][1] *= -1;
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
        simpleGraphicsPipeline.Destroy();
        simpleRenderPass.Destroy();
        fixedFunctions.DestroyPipelineLayout();
        _def_Swapchain.Destroy();
    }


private:

    void OnStart() override {

        simpleRenderPass.set_clear_color(0.0f, 0.0f, 0.0f);
        auto& cmdBuffers = simpleCommandBuffer.GetBuffers();
        auto framebuffers = simpleFrameBuffer.GetFramebuffers();
        auto descriptorSets = helloTriangleUBO.GetSets();

        for (int i = 0; i < cmdBuffers.size(); i++) {

#ifdef _WIN32
            OPTICK_GPU_CONTEXT(cmdBuffers[i]);
            OPTICK_GPU_EVENT("Recording cmd buffers");
#endif
            simpleCommandBuffer.RecordBuffer(cmdBuffers[i]);
            simpleRenderPass.begin_pass(cmdBuffers[i], framebuffers[i], _def_Swapchain.GetSwapExtent());

            simpleGraphicsPipeline.Bind(cmdBuffers[i]);

            // Bind the appropriate descriptor sets
            vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, fixedFunctions.GetPipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);

            // Bind the push constants
            modelPCData.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.03f));
            vkCmdPushConstants(cmdBuffers[i], fixedFunctions.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ModelPushConstant), &modelPCData);

            // testModel.bindBuffers(cmdBuffers[i]);

            // testModel.draw(cmdBuffers[i]);

            simpleRenderPass.end_pass(cmdBuffers[i]);
            simpleCommandBuffer.EndRecordingBuffer(cmdBuffers[i]);
        }

        submissionCommandBuffers.clear();
        submissionCommandBuffers.push_back(simpleCommandBuffer);
    }

    void OnRender() override {
        ZoneScopedC(0xffa500);
        //OPTICK_EVENT();
    }
};

VULF_MAIN(glTFLoadingDemo)
#endif
