#pragma once

#include "VKShader.h"
#include "VKFixedPipelineFuncs.h"
#include "VKRenderPass.h"

#include "VKCmdBuffer.h"

class VKGraphicsPipeline
{
public:
    VKGraphicsPipeline() = default;
    void Create(std::vector<VkPipelineShaderStageCreateInfo> shaderStages,
    VKFixedPipelineFuncs& fixedfunctions,
    VkRenderPass& renderpass);
    void Bind(VKCmdBuffer& cmdBuffers);
    void Destroy();
private:
    VkPipeline m_GraphicsPipeline;
};
