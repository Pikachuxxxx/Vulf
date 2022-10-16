#pragma once

#include "Shader.h"
#include "FixedPipelineFuncs.h"
#include "RenderPass.h"

#include "CmdBuffer.h"

class ComputePipeline
{
public:
    ComputePipeline() = default;
    void Init(Shader computeShader, VkPipelineLayout layout);
    void Destroy();

    void bind(VkCommandBuffer& cmdBuffers);
private:
    VkPipeline m_Pipeline;
};
