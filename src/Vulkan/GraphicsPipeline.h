#pragma once

#include "Shader.h"
#include "FixedPipelineFuncs.h"
#include "RenderPass.h"

#include "CmdBuffer.h"

class GraphicsPipeline
{
public:
    GraphicsPipeline() = default;
    void Create(std::vector<VkPipelineShaderStageCreateInfo> shaderStages,
    FixedPipelineFuncs& fixedfunctions,
    VkRenderPass& renderpass);
    void Bind(VkCommandBuffer& cmdBuffers);
    void Destroy();
private:
    VkPipeline m_GraphicsPipeline;
};
