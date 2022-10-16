#include "ComputePipeline.h"

#include "Device.h"

#include "../utils/VulkanCheckResult.h"

void ComputePipeline::Init(Shader computeShader, VkPipelineLayout layout)
{
    VkComputePipelineCreateInfo pipelineCI = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .stage = computeShader.GetShaderStageInfo(),
        .layout = layout
    };

    if (VK_CALL(vkCreateComputePipelines(VKDEVICE, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &m_Pipeline)))
        throw std::runtime_error("Cannot create Compute pipeline!");
    else VK_LOG_SUCCESS("Compute Pipeline succesfully created!");
}

void ComputePipeline::Destroy()
{
    vkDestroyPipeline(VKDEVICE, m_Pipeline, nullptr);
}

void ComputePipeline::bind(VkCommandBuffer& cmdBuffers)
{
    vkCmdBindPipeline(cmdBuffers, VK_PIPELINE_BIND_POINT_COMPUTE, m_Pipeline);
}
