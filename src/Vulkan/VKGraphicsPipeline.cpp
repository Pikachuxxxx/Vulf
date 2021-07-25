#include "VKGraphicsPipeline.h"

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"

void VKGraphicsPipeline::Create(std::vector<VkPipelineShaderStageCreateInfo> shaderStages, VKFixedPipelineFuncs& fixedfunctions, VkRenderPass& renderpass)
{
    VkGraphicsPipelineCreateInfo graphicsCI{};
    graphicsCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsCI.stageCount = shaderStages.size();
    graphicsCI.pStages = shaderStages.data();
    graphicsCI.pVertexInputState    = &fixedfunctions.GetVertexInputSCI();
    graphicsCI.pInputAssemblyState  = &fixedfunctions.GetInputAssemblySCI();
    graphicsCI.pViewportState       = &fixedfunctions.GetViewportStateCI();
    graphicsCI.pRasterizationState  = &fixedfunctions.GetRazterizerSCI();
    graphicsCI.pMultisampleState    = &fixedfunctions.GetMultiSampleSCI();
    graphicsCI.pDepthStencilState   = &fixedfunctions.GetDepthStencilSCI();
    graphicsCI.pColorBlendState     = &fixedfunctions.GetColorBlendSCI();
    graphicsCI.pDynamicState        = nullptr;
    graphicsCI.layout               = fixedfunctions.GetPipelineLayout();
    graphicsCI.renderPass = renderpass;
    graphicsCI.subpass = 0;
    graphicsCI.basePipelineHandle = VK_NULL_HANDLE;
    graphicsCI.basePipelineIndex = -1;

    if(VK_CALL(vkCreateGraphicsPipelines(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), VK_NULL_HANDLE, 1, &graphicsCI, nullptr, &m_GraphicsPipeline)))
        throw std::runtime_error("Cannot create Graphics pipeline!");
    else VK_LOG_SUCCESS("Graphics Pipeline succesfully crreated!");
}

void VKGraphicsPipeline::Destroy()
{
    vkDestroyPipeline(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_GraphicsPipeline, nullptr);
}

void VKGraphicsPipeline::Bind(VKCmdBuffer& cmdBuffers)
{
    VK_LOG("Binding graphics pipeline!");
    for (const auto& cmdBuffer : cmdBuffers.GetBuffers()) {
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
    }
}
