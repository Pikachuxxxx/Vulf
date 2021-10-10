#include <iostream>

#include <VulfBase.h>
#include "utils/VulkanCheckResult.h"

// Load the Instance extensions/layers and device Extensions
std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

std::vector<const char*> instanceExtensions = {
    "VK_KHR_get_physical_device_properties2"
};

std::vector<const char*> deviceExtensions = {
   VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if (__APPLE__)
   "VK_KHR_portability_subset"
#endif
};

#include "VulfBase.h"

class VulfHelloTriangle : public Vulf::VulfBase
{
public:
    VulfHelloTriangle() : VulfBase("Hello Triangle") {}

    ~VulfHelloTriangle() {
        _def_CommandPool.Destroy();
        CleanUpPipeline();
        defaultVertShader.DestroyModule();
        defaultFragShader.DestroyModule();
        VKLogicalDevice::GetDeviceManager()->Destroy();
        VKInstance::GetInstanceManager()->Destroy();
    }

// Types
private:
    struct ModelPushConstant{
        alignas(16) glm::mat4 model;
    }modelPCData;

private:
    using ShaderStage = std::vector<VkPipelineShaderStageCreateInfo>;
    // Shaders
    VKShader                defaultVertShader;
    VKShader                defaultFragShader;
    ShaderStage             defaultShaders;

    // Buffers
    VKVertexBuffer          helloTriangleVBO;

    VKFixedPipelineFuncs    fixedFunctions;

    VKRenderPass            simpleRenderPass;

    VKGraphicsPipeline      simpleGraphicsPipeline;

    VkPushConstantRange     modelPushConstant;

    VKDepthImage            depthImage;

    VKFramebuffer           simpleFrameBuffer;

    VKCmdBuffer             simpleCommandBuffer;

private:
    void LoadShaders() override {

        // Default shaders
        defaultVertShader.CreateShader((SHADER_BINARY_DIR) + std::string("/defaultVert.spv"), ShaderType::VERTEX_SHADER);
        defaultFragShader.CreateShader((SHADER_BINARY_DIR) + std::string("/defaultFrag.spv"), ShaderType::FRAGMENT_SHADER);
        defaultShaders.push_back(defaultVertShader.GetShaderStageInfo());
        defaultShaders.push_back(defaultFragShader.GetShaderStageInfo());
    }

    void BuildBufferResource() override {
        // Triangle vertices and indices
        helloTriangleVBO.Create(rainbowTriangleVertices, _def_CommandPool);
    }

    void BuildTextureResources() override {
        // default
        depthImage.CreateDepthImage(_def_Swapchain.GetSwapExtent().width, _def_Swapchain.GetSwapExtent().height, _def_CommandPool);
    }

    void BuildFixedPipeline() override {
        // Create the push contants
        modelPushConstant.stageFlags    = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        modelPushConstant.offset        = 0;
        modelPushConstant.size          = sizeof(ModelPushConstant);

        fixedFunctions.SetFixedPipelineStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, _def_Swapchain.GetSwapExtent(), false);
        fixedFunctions.SetPipelineLayout(VK_NULL_HANDLE, modelPushConstant);
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

    void UpdateBuffers() override {

    }

    void CleanUpPipeline() override {
        
        simpleFrameBuffer.Destroy();
        depthImage.Destroy();
        helloTriangleVBO.Destroy();
        simpleGraphicsPipeline.Destroy();
        simpleRenderPass.Destroy();
        fixedFunctions.DestroyPipelineLayout();
        _def_Swapchain.Destroy();
    }


private:

    void OnRender() override
    {
        simpleRenderPass.SetClearColor(0.0f, 0.0f, 0.0f);
        auto cmdBuffers = simpleCommandBuffer.GetBuffers();
        auto framebuffers = simpleFrameBuffer.GetFramebuffers();

        //int i = GetNextImageIndex();
        for (int i = 0; i <  cmdBuffers.size(); i++) {
            simpleCommandBuffer.RecordBuffer(cmdBuffers[i]);
            simpleRenderPass.BeginRenderPass(cmdBuffers[i], framebuffers[i], _def_Swapchain.GetSwapExtent());

            simpleGraphicsPipeline.Bind(cmdBuffers[i]);

            // Bind the push constants
            modelPCData.model = glm::rotate(glm::mat4(1.0f), (float)glm::radians(90.0f * 2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            vkCmdPushConstants(cmdBuffers[i], fixedFunctions.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ModelPushConstant), &modelPCData);

            helloTriangleVBO.Bind(cmdBuffers[i]);

            vkCmdDraw(cmdBuffers[i], rainbowTriangleVertices.size(), 1, 0, 0);

            simpleRenderPass.EndRenderPass(cmdBuffers[i]);
            simpleCommandBuffer.EndRecordingBuffer(cmdBuffers[i]);
        }

            submissionCommandBuffers.clear();
            submissionCommandBuffers.push_back(simpleCommandBuffer);
    }
};

VULF_MAIN(HelloTriangle)
